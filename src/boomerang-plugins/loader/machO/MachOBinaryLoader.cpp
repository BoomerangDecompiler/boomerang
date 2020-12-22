#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MachOBinaryLoader.h"

#include "macho-apple.h"
#include "nlist.h"
#include "objc/objc-class.h"
#include "objc/objc-runtime.h"

#include "boomerang/core/plugin/Plugin.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbol.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/util/log/Log.h"

#include <QBuffer>
#include <QFile>

#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <stdexcept>


#define DEBUG_MACHO_LOADER 0

#if DEBUG_MACHO_LOADER
#    define DEBUG_PRINT(...)                                                                       \
        do {                                                                                       \
            LOG_VERBOSE(__VA_ARGS__);                                                              \
        } while (false)
#else
#    define DEBUG_PRINT(...)
#endif

// #define DEBUG_MACHO_LOADER
// #define DEBUG_MACHO_LOADER_OBJC

MachOBinaryLoader::MachOBinaryLoader(Project *project)
    : IFileLoader(project)
{
    machine    = Machine::PPC;
    swap_bytes = false;
}


MachOBinaryLoader::~MachOBinaryLoader()
{
}


void MachOBinaryLoader::initialize(BinaryFile *file, BinarySymbolTable *symbols)
{
    Image   = file->getImage();
    Symbols = symbols;
}


void MachOBinaryLoader::close()
{
    unload();
}


Address MachOBinaryLoader::getEntryPoint()
{
    return entrypoint;
}


Address MachOBinaryLoader::getMainEntryPoint()
{
    auto symbol = Symbols->findSymbolByName("main");

    if (symbol) {
        return symbol->getLocation();
    }

    symbol = Symbols->findSymbolByName("_main");

    if (symbol) {
        return symbol->getLocation();
    }

    return Address::INVALID;
}


bool MachOBinaryLoader::loadFromMemory(QByteArray &img)
{
    QBuffer fp(&img);

    if (!fp.open(QFile::ReadOnly)) {
        return false;
    }

    unsigned int imgoffs = 0;

    unsigned char *magic = reinterpret_cast<uint8_t *>(img.data());
    struct mach_header *header; // The Mach-O header

    if (Util::testMagic(magic, { 0xca, 0xfe, 0xba, 0xbe })) {
        const int nimages = Util::readDWord(magic + 4, Endian::Big);
        DEBUG_PRINT("Binary is universal with %1 images", nimages);

        for (int i = 0; i < nimages; i++) {
            int fbh              = 8 + i * 5 * 4;
            unsigned int cputype = Util::readDWord(magic + fbh + 0, Endian::Big);
            unsigned int offset  = Util::readDWord(magic + fbh + 8, Endian::Big);
            DEBUG_PRINT("cputype:    %1", cputype);
            DEBUG_PRINT("cpusubtype: %1", BE4(fbh + 4));
            DEBUG_PRINT("offset:     %1", BE4(fbh + 8));
            DEBUG_PRINT("size:       %1", BE4(fbh + 12));
            DEBUG_PRINT("pad:        %1", BE4(fbh + 16));

            if (cputype == 0x7) { // i386
                imgoffs = offset;
            }
        }
    }

    header = reinterpret_cast<mach_header *>(img.data() + imgoffs); // new mach_header;
    // fp.read((char *)header, sizeof(mach_header));

    if ((header->magic != MH_MAGIC) && (READ4_BE(header->magic) != MH_MAGIC)) {
        LOG_ERROR("Error loading file, bad Mach-O magic");
        return false;
    }

    // check for swapped bytes
    swap_bytes = (READ4_BE(header->magic) == MH_MAGIC);

    // Determine CPU type
    if (BMMH(header->cputype) == 0x07) {
        machine = Machine::X86;
    }
    else {
        machine = Machine::PPC;
    }

    sections.clear();
    std::vector<segment_command> segments;
    std::vector<nlist> symbols;
    // uint32_t startundef, nundef;
    // uint32_t  startlocal, nlocal,ndef, startdef;
    std::vector<section> stubs_sects;

    std::vector<char> strtbl;
    std::vector<DWord> indirectsymtbl;

    Address objc_symbols       = Address::INVALID;
    Address objc_modules       = Address::INVALID;
    Address objc_strings       = Address::INVALID;
    Address objc_refs          = Address::INVALID;
    unsigned objc_modules_size = 0;

    fp.seek(imgoffs + sizeof(*header));

    for (unsigned i = 0; i < BMMH(header->ncmds); i++) {
        load_command cmd;
        long pos = fp.pos();
        fp.read(reinterpret_cast<char *>(&cmd), sizeof(load_command));
        fp.seek(pos);

        switch (BMMH(cmd.cmd)) {
        case LC_SEGMENT: {
            segment_command seg;
            fp.read(reinterpret_cast<char *>(&seg), sizeof(seg));
            segments.push_back(seg);
            DEBUG_PRINT("seg addr %1 size %2 fileoff %3 filesize %4 flags %5", BMMH(seg.vmaddr),
                        BMMH(seg.vmsize), BMMH(seg.fileoff), BMMH(seg.filesize), BMMH(seg.flags));

            for (DWord n = 0; n < BMMH(seg.nsects); n++) {
                section sect;
                fp.read(reinterpret_cast<char *>(&sect), sizeof(sect));
                sections.push_back(sect);
                DEBUG_PRINT("    sectname %1 segname %2 addr %3 size %4 flags %5", sect.sectname,
                            sect.segname, BMMH(sect.addr), BMMH(sect.size), BMMH(sect.flags));

                if ((BMMH(sect.flags) & SECTION_TYPE) == S_SYMBOL_STUBS) {
                    stubs_sects.push_back(sect);
                    DEBUG_PRINT("        symbol stubs section, start index %1, stub size %2",
                                BMMH(sect.reserved1), BMMH(sect.reserved2));
                }

                if (!strcmp(sect.sectname, SECT_OBJC_SYMBOLS)) {
                    assert(objc_symbols == Address::INVALID);
                    objc_symbols = Address(BMMH(sect.addr));
                }

                if (!strcmp(sect.sectname, SECT_OBJC_MODULES)) {
                    assert(objc_modules == Address::INVALID);
                    objc_modules      = Address(BMMH(sect.addr));
                    objc_modules_size = BMMH(sect.size);
                }

                if (!strcmp(sect.sectname, SECT_OBJC_STRINGS)) {
                    assert(objc_strings == Address::INVALID);
                    objc_strings = Address(BMMH(sect.addr));
                }

                if (!strcmp(sect.sectname, SECT_OBJC_REFS)) {
                    assert(objc_refs == Address::INVALID);
                    objc_refs = Address(BMMH(sect.addr));
                }
            }
        } break;

        case LC_SYMTAB: {
            symtab_command syms;
            fp.read(reinterpret_cast<char *>(&syms), sizeof(syms));
            fp.seek(imgoffs + BMMH(syms.stroff));
            strtbl.resize(BMMH(syms.strsize));
            fp.read(&strtbl[0], BMMH(syms.strsize));
            fp.seek(imgoffs + BMMH(syms.symoff));

            for (unsigned n = 0; n < BMMH(syms.nsyms); n++) {
                nlist sym;
                fp.read(reinterpret_cast<char *>(&sym), sizeof(sym));
                symbols.push_back(sym);
                // DEBUG_PRINT(stdout, "got sym %s flags %x value %x\n", strtbl +
                // BMMH(sym.n_un.n_strx), sym.n_type, BMMH(sym.n_value));
            }

            DEBUG_PRINT("symtab contains %1 symbols", BMMH(syms.nsyms));
        } break;

        case LC_DYSYMTAB: {
            struct dysymtab_command syms;
            fp.read(reinterpret_cast<char *>(&syms), sizeof(syms));
            DEBUG_PRINT("dysymtab local %1 %2 defext %3 %4 undef %5 %6", BMMH(syms.ilocalsym),
                        BMMH(syms.nlocalsym), BMMH(syms.iextdefsym), BMMH(syms.nextdefsym),
                        BMMH(syms.iundefsym), BMMH(syms.nundefsym));

            // TODO: find uses for values below
            // startlocal = BMMH(syms.ilocalsym);
            // nlocal = BMMH(syms.nlocalsym);
            // startdef = BMMH(syms.iextdefsym);
            // ndef = BMMH(syms.nextdefsym);
            // startundef = BMMH(syms.iundefsym);
            // nundef = BMMH(syms.nundefsym);

            DEBUG_PRINT("dysymtab has %1 indirect symbols: ", BMMH(syms.nindirectsyms));
            indirectsymtbl.resize(BMMH(syms.nindirectsyms));
            fp.seek(imgoffs + BMMH(syms.indirectsymoff));
            fp.read(reinterpret_cast<char *>(&indirectsymtbl[0]),
                    indirectsymtbl.size() * sizeof(DWord));

            for (unsigned j = 0; j < BMMH(syms.nindirectsyms); j++) {
                DEBUG_PRINT("  %1 ", BMMH(indirectsymtbl[j]));
            }
        } break;

        default:
            DEBUG_PRINT("Not handled load command %1", BMMH(cmd.cmd));
            // yep, there's lots of em
            break;
        }

        fp.seek(pos + BMMH(cmd.cmdsize));
    }

    segment_command *lowest = &segments[0], *highest = &segments[0];

    for (unsigned i = 1; i < segments.size(); i++) {
        if (BMMH(segments[i].vmaddr) < BMMH(lowest->vmaddr)) {
            lowest = &segments[i];
        }

        if (BMMH(segments[i].vmaddr) > BMMH(highest->vmaddr)) {
            highest = &segments[i];
        }
    }

    loaded_addr = Address(BMMH(lowest->vmaddr));
    loaded_size = BMMH(highest->vmaddr) - BMMH(lowest->vmaddr) + BMMH(highest->vmsize);

    base = reinterpret_cast<char *>(malloc(loaded_size));

    if (!base) {
        LOG_ERROR("Cannot allocate memory for copy of image");
        return false;
    }

    for (unsigned i = 0; i < segments.size(); i++) {
        fp.seek(imgoffs + BMMH(segments[i].fileoff));
        Address a    = Address(BMMH(segments[i].vmaddr));
        unsigned sz  = BMMH(segments[i].vmsize);
        unsigned fsz = BMMH(segments[i].filesize);
        memset(base + a.value() - loaded_addr.value(), 0, sz);
        fp.read(base + a.value() - loaded_addr.value(), fsz);
        DEBUG_PRINT("loaded segment %1 %2 in mem %3 in file", a.value(), sz, fsz);

        QString name        = QByteArray(segments[i].segname, 17);
        BinarySection *sect = Image->createSection(
            name, Address(BMMH(segments[i].vmaddr)),
            Address(BMMH(segments[i].vmaddr) + BMMH(segments[i].vmsize)));
        assert(sect);
        sect->setHostAddr(HostAddress(base) + BMMH(segments[i].vmaddr) - loaded_addr);
        assert(sect->getHostAddr() + sect->getSize() <= HostAddress(base) + loaded_size);

        unsigned long l = BMMH(segments[i].initprot);
        sect->setBss(false); // TODO
        sect->setEndian((machine == Machine::PPC) ? Endian::Big : Endian::Little);
        sect->setCode((l & VM_PROT_EXECUTE) != 0);
        sect->setData((l & VM_PROT_READ) != 0);
        sect->setReadOnly((l & VM_PROT_WRITE) == 0);

        for (size_t s_idx = 0; s_idx < sections.size(); s_idx++) {
            if (strcmp(sections[s_idx].segname, segments[i].segname) != 0) {
                continue;
            }

            if ((0 == strcmp(sections[s_idx].sectname, "__cfstring")) ||
                (0 == strcmp(sections[s_idx].sectname, "__cstring"))) {
                sect->setAttributeForRange(
                    "StringsSection", true, Address(BMMH(sections[s_idx].addr)),
                    Address(BMMH(sections[s_idx].addr) + BMMH(sections[s_idx].size)));
            }

            sect->setAttributeForRange(
                "ReadOnly", (BMMH(sections[i].flags) & VM_PROT_WRITE) ? true : false,
                Address(BMMH(sections[s_idx].addr)),
                Address(BMMH(sections[s_idx].addr) + BMMH(sections[s_idx].size)));
        }

        DEBUG_PRINT("loaded segment %1 %2 in mem %3 in file code=%4 data=%5 readonly=%6", a.value(),
                    sz, fsz, sect->isCode(), sect->isData(), sect->isReadOnly());
    }

    // process stubs_sects
    for (unsigned j = 0; j < stubs_sects.size(); j++) {
        for (unsigned i = 0; i < BMMH(stubs_sects[j].size) / BMMH(stubs_sects[j].reserved2); i++) {
            unsigned startidx = BMMH(stubs_sects[j].reserved1);
            unsigned symbol   = BMMH(indirectsymtbl[startidx + i]);
            Address addr = Address(BMMH(stubs_sects[j].addr) + i * BMMH(stubs_sects[j].reserved2));

            DEBUG_PRINT("stub for %1 at %2", strtbl + BMMH(symbols[symbol].n_un.n_strx),
                        addr.value());
            char *name = &strtbl.at(BMMH(symbols[symbol].n_un.n_strx));

            if (*name == '_') { // we want printf not _printf
                name++;
            }

            BinarySymbol *sym = Symbols->createSymbol(addr, name);
            sym->setAttribute("Function", true);
            sym->setAttribute("Imported", true);
        }
    }

    // process the remaining symbols
    for (unsigned i = 0; i < symbols.size(); i++) {
        char *name = &strtbl.at(BMMH(symbols[i].n_un.n_strx));

        if ((BMMH(symbols[i].n_un.n_strx) != 0) && (BMMH(symbols[i].n_value) != 0) &&
            (*name != 0)) {
            uint8_t sym_type = symbols[i].n_type;
            bool is_stab     = (sym_type & N_STAB) != 0;

            if (is_stab) {
                continue; // TODO: handle stab symbols !
            }

            DEBUG_PRINT("symbol %s at %x type %x\n", name, BMMH(symbols[i].n_value),
                        sym_type & N_TYPE);

            if (*name == '_') { // we want main not _main
                name++;
            }

            Symbols->createSymbol(Address(BMMH(symbols[i].n_value)), name);
        }
    }

    // process objective-c section
    if (objc_modules != Address::INVALID) {
        DEBUG_PRINT("Processing objective-c section");

        for (unsigned i = 0; i < objc_modules_size;) {
            struct objc_module *module = reinterpret_cast<struct objc_module *>(
                (HostAddress(base) + objc_modules - loaded_addr + i).value());
            char *name    = reinterpret_cast<char *>(intptr_t(base) + BMMH(module->name) -
                                                  loaded_addr.value());
            Symtab symtab = reinterpret_cast<Symtab>(
                (HostAddress(base) + BMMH(module->symtab) - loaded_addr).value());

#ifdef DEBUG_MACHO_LOADER_OBJC
            LOG_MSG("module %1 (%2 classes)", name, BMMHW(symtab->cls_def_cnt));
#endif
            ObjcModule *m = &modules[name];
            m->name       = name;

            for (unsigned j = 0; j < BMMHW(symtab->cls_def_cnt); j++) {
                struct objc_class *def = reinterpret_cast<struct objc_class *>(
                    base + BMMH(symtab->defs[j]) - loaded_addr.value());
                char *_name = reinterpret_cast<char *>(Address::value_type(base) + BMMH(def->name) -
                                                       loaded_addr.value());

#ifdef DEBUG_MACHO_LOADER_OBJC
                LOG_MSG("  class %1", name);
#endif

                ObjcClass *cl                = &m->classes[_name];
                cl->name                     = _name;
                struct objc_ivar_list *ivars = reinterpret_cast<struct objc_ivar_list *>(
                    base + BMMH(def->ivars) - loaded_addr.value());

                for (unsigned k = 0; k < static_cast<unsigned int>(BMMH(ivars->ivar_count)); k++) {
                    struct objc_ivar *ivar = &ivars->ivar_list[k];
                    char *name2            = reinterpret_cast<char *>(
                        Address::value_type(base) + BMMH(ivar->ivar_name) - loaded_addr.value());
                    char *types = reinterpret_cast<char *>(
                        Address::value_type(base) + BMMH(ivar->ivar_type) - loaded_addr.value());

#ifdef DEBUG_MACHO_LOADER_OBJC
                    LOG_MSG("    ivar %1 %2 %3", name, types, BMMH(ivar->ivar_offset));
#endif

                    ObjcIvar *iv = &cl->ivars[name2];
                    iv->name     = name2;
                    iv->type     = types;
                    iv->offset   = BMMH(ivar->ivar_offset);
                }

                // this is weird, why is it defined as a ** in the struct but used as a * in otool?
                struct objc_method_list *methods = reinterpret_cast<struct objc_method_list *>(
                    intptr_t(base) + BMMH(def->methodLists) - loaded_addr.value());

                for (unsigned k = 0; k < static_cast<unsigned int>(BMMH(methods->method_count));
                     k++) {
                    struct objc_method *method = &methods->method_list[k];
                    char *name3                = reinterpret_cast<char *>(
                        intptr_t(base) + BMMH(method->method_name) - loaded_addr.value());
                    char *types = reinterpret_cast<char *>(
                        intptr_t(base) + BMMH(method->method_types) - loaded_addr.value());

#ifdef DEBUG_MACHO_LOADER_OBJC
                    LOG_MSG("    method %1 %2 %3", name, types, BMMH((void *)method->method_imp));
#endif
                    ObjcMethod *me = &cl->methods[name3];
                    me->name       = name3;
                    me->types      = types;
                    me->addr       = Address(BMMH(method->method_imp));
                }
            }

            i += BMMH(module->size);
        }
    }

    entrypoint = getMainEntryPoint();
    return true;
}


int MachOBinaryLoader::canLoad(QIODevice &dev) const
{
    Byte buf[8];

    dev.read(reinterpret_cast<char *>(buf), sizeof(buf));

    if (((buf[0] == 0xfe) && (buf[1] == 0xed) && (buf[2] == 0xfa) && (buf[3] == 0xce)) ||
        ((buf[0] == 0xce) && (buf[1] == 0xfa) && (buf[2] == 0xed) && (buf[3] == 0xfe)) ||
        ((buf[0] == 0xca) && (buf[1] == 0xfe) && (buf[2] == 0xba) && (buf[3] == 0xbe))) {
        /* Mach-O Mac OS-X binary */
        return 4;
    }

    return 0;
}


// Clean up and unload the binary image
void MachOBinaryLoader::unload()
{
}


SWord MachOBinaryLoader::machORead2(const void *ps) const
{
    return Util::readWord(ps, (machine == Machine::PPC) ? Endian::Big : Endian::Little);
}


DWord MachOBinaryLoader::machORead4(const void *pi) const
{
    return Util::readDWord(pi, (machine == Machine::PPC) ? Endian::Big : Endian::Little);
}


int32_t MachOBinaryLoader::BMMH(int32_t x)
{
    if (swap_bytes) {
        return READ4_BE(x);
    }
    else {
        return x;
    }
}


DWord MachOBinaryLoader::BMMH(DWord x)
{
    if (swap_bytes) {
        return READ4_BE(x);
    }
    else {
        return x;
    }
}


SWord MachOBinaryLoader::BMMHW(SWord x)
{
    if (swap_bytes) {
        return READ2_BE(x);
    }
    else {
        return x;
    }
}


LoadFmt MachOBinaryLoader::getFormat() const
{
    return LoadFmt::MACHO;
}


Machine MachOBinaryLoader::getMachine() const
{
    return machine;
}


bool MachOBinaryLoader::isLibrary() const
{
    return false;
}


DWord MachOBinaryLoader::getDelta()
{
    // Stupid function anyway: delta depends on section
    // This should work for the header only
    //    return (DWord)base - LMMH(m_peHeader->Imagebase);
    return (HostAddress(base) - loaded_addr).value();
}

BOOMERANG_DEFINE_PLUGIN(PluginType::FileLoader, MachOBinaryLoader, "Mach-O loader plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
