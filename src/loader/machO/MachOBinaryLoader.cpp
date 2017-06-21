/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file MachOBinaryLoader.cpp
 * \brief This file contains the implementation of the class MachOBinaryFile.
 *
 *    This file implements the class MachOBinaryFile, derived from class
 *    BinaryFile. See MachOBinaryFile.h and BinaryFile.h for details.
 */

#include "MachOBinaryLoader.h"

#include "core/BinaryFile.h"
#include "include/IBoomerang.h"
#include "db/IBinaryImage.h"
#include "db/IBinarySymbols.h"
#include "include/config.h"
#include "nlist.h"
#include "macho-apple.h"

#include "objc/objc-class.h"
#include "objc/objc-runtime.h"

#include <cstdarg>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <QFile>
#include <QBuffer>
#define DEBUG_MACHO_LOADER    0

#if DEBUG_MACHO_LOADER
#  define DEBUG_PRINT(...)										   \
	do { if (DEBUG_MACHO_LOADER) { fprintf(stderr, __VA_ARGS__); } \
	} while (0)
#else
#  define DEBUG_PRINT(...)
#endif

namespace
{
struct SectionParam
{
	QString Name;
	ADDRESS from;
	size_t  Size;
	ADDRESS ImageAddress;
	bool    Bss, Code, Data, ReadOnly;
};
}

// #define DEBUG_MACHO_LOADER
// #define DEBUG_MACHO_LOADER_OBJC

MachOBinaryLoader::MachOBinaryLoader()
{
	machine    = Machine::PPC;
	swap_bytes = false;
}


MachOBinaryLoader::~MachOBinaryLoader()
{
}


void MachOBinaryLoader::initialize(IBinaryImage *image, IBinarySymbolTable *symbols)
{
	Image   = image;
	Symbols = symbols;
}


void MachOBinaryLoader::close()
{
	unload();
}


ADDRESS MachOBinaryLoader::getEntryPoint()
{
	return entrypoint;
}


ADDRESS MachOBinaryLoader::getMainEntryPoint()
{
	auto symbol = Symbols->find("main");

	if (symbol) {
		return symbol->getLocation();
	}

	symbol = Symbols->find("_main");

	if (symbol) {
		return symbol->getLocation();
	}

	return NO_ADDRESS;
}


#define BE4(x)    ((magic[(x)] << 24) | (magic[(x) + 1] << 16) | (magic[(x) + 2] << 8) | (magic[(x) + 3]))

bool MachOBinaryLoader::loadFromMemory(QByteArray& img)
{
	QBuffer fp(&img);

	if (!fp.open(QFile::ReadOnly)) {
		return false;
	}

	unsigned int imgoffs = 0;

	unsigned char      *magic = (uint8_t *)img.data();
	struct mach_header *header; // The Mach-O header

	if ((magic[0] == 0xca) && (magic[1] == 0xfe) && (magic[2] == 0xba) && (magic[3] == 0xbe)) {
		int nimages = BE4(4);
		DEBUG_PRINT("binary is universal with %d images\n", nimages);

		for (int i = 0; i < nimages; i++) {
			int          fbh     = 8 + i * 5 * 4;
			unsigned int cputype = BE4(fbh);
			unsigned int offset  = BE4(fbh + 8);
			DEBUG_PRINT("cputype: %08x\n", cputype);
			DEBUG_PRINT("cpusubtype: %08x\n", BE4(fbh + 4));
			DEBUG_PRINT("offset: %08x\n", BE4(fbh + 8));
			DEBUG_PRINT("size: %08x\n", BE4(fbh + 12));
			DEBUG_PRINT("pad: %08x\n", BE4(fbh + 16));

			if (cputype == 0x7) { // i386
				imgoffs = offset;
			}
		}
	}

	header = (mach_header *)(img.data() + imgoffs);// new mach_header;
	// fp.read((char *)header, sizeof(mach_header));

	if ((header->magic != MH_MAGIC) && (_BMMH(header->magic) != MH_MAGIC)) {
		qWarning() << "error loading file, bad Mach-O magic";
		return false;
	}

	// check for swapped bytes
	swap_bytes = (_BMMH(header->magic) == MH_MAGIC);

	// Determine CPU type
	if (BMMH(header->cputype) == 0x07) {
		machine = Machine::PENTIUM;
	}
	else {
		machine = Machine::PPC;
	}

	sections.clear();
	std::vector<segment_command> segments;
	std::vector<nlist>           symbols;
	// uint32_t startundef, nundef;
	// uint32_t  startlocal, nlocal,ndef, startdef;
	std::vector<section> stubs_sects;
	
	char                 *strtbl = nullptr;
	unsigned             *indirectsymtbl = nullptr;
	ADDRESS              objc_symbols = NO_ADDRESS, objc_modules = NO_ADDRESS, objc_strings = NO_ADDRESS, objc_refs = NO_ADDRESS;
	unsigned             objc_modules_size = 0;

	fp.seek(imgoffs + sizeof(*header));

	for (unsigned i = 0; i < BMMH(header->ncmds); i++) {
		load_command cmd;
		long         pos = fp.pos();
		fp.read((char *)&cmd, sizeof(load_command));
		fp.seek(pos);

		switch (BMMH(cmd.cmd))
		{
		case LC_SEGMENT:
			{
				segment_command seg;
				fp.read((char *)&seg, sizeof(seg));
				segments.push_back(seg);
				DEBUG_PRINT("seg addr %x size %i fileoff %x filesize %i flags %x\n", BMMH(seg.vmaddr), BMMH(seg.vmsize),
							BMMH(seg.fileoff), BMMH(seg.filesize), BMMH(seg.flags));

				for (unsigned n = 0; n < BMMH(seg.nsects); n++) {
					section sect;
					fp.read((char *)&sect, sizeof(sect));
					sections.push_back(sect);
					DEBUG_PRINT("    sectname %s segname %s addr %x size %i flags %x\n", sect.sectname, sect.segname,
								BMMH(sect.addr), BMMH(sect.size), BMMH(sect.flags));

					if ((BMMH(sect.flags) & SECTION_TYPE) == S_SYMBOL_STUBS) {
						stubs_sects.push_back(sect);
						DEBUG_PRINT("        symbol stubs section, start index %i, stub size %i\n",
									BMMH(sect.reserved1), BMMH(sect.reserved2));
					}

					if (!strcmp(sect.sectname, SECT_OBJC_SYMBOLS)) {
						assert(objc_symbols == NO_ADDRESS);
						objc_symbols = BMMH(sect.addr);
					}

					if (!strcmp(sect.sectname, SECT_OBJC_MODULES)) {
						assert(objc_modules == NO_ADDRESS);
						objc_modules      = BMMH(sect.addr);
						objc_modules_size = BMMH(sect.size);
					}

					if (!strcmp(sect.sectname, SECT_OBJC_STRINGS)) {
						assert(objc_strings == NO_ADDRESS);
						objc_strings = BMMH(sect.addr);
					}

					if (!strcmp(sect.sectname, SECT_OBJC_REFS)) {
						assert(objc_refs == NO_ADDRESS);
						objc_refs = BMMH(sect.addr);
					}
				}
			}
			break;

		case LC_SYMTAB:
			{
				symtab_command syms;
				fp.read((char *)&syms, sizeof(syms));
				fp.seek(imgoffs + BMMH(syms.stroff));
				strtbl = new char[BMMH(syms.strsize)];
				fp.read(strtbl, BMMH(syms.strsize));
				fp.seek(imgoffs + BMMH(syms.symoff));

				for (unsigned n = 0; n < BMMH(syms.nsyms); n++) {
					nlist sym;
					fp.read((char *)&sym, sizeof(sym));
					symbols.push_back(sym);
					// DEBUG_PRINT(stdout, "got sym %s flags %x value %x\n", strtbl + BMMH(sym.n_un.n_strx), sym.n_type, BMMH(sym.n_value));
				}

				DEBUG_PRINT("symtab contains %i symbols\n", BMMH(syms.nsyms));
			}
			break;

		case LC_DYSYMTAB:
			{
				struct dysymtab_command syms;
				fp.read((char *)&syms, sizeof(syms));
				DEBUG_PRINT("dysymtab local %i %i defext %i %i undef %i %i\n", BMMH(syms.ilocalsym),
							BMMH(syms.nlocalsym), BMMH(syms.iextdefsym), BMMH(syms.nextdefsym), BMMH(syms.iundefsym),
							BMMH(syms.nundefsym));
				// TODO: find uses for values below
				// startlocal = BMMH(syms.ilocalsym);
				// nlocal = BMMH(syms.nlocalsym);
				// startdef = BMMH(syms.iextdefsym);
				// ndef = BMMH(syms.nextdefsym);
				// startundef = BMMH(syms.iundefsym);
				// nundef = BMMH(syms.nundefsym);

				DEBUG_PRINT("dysymtab has %i indirect symbols: ", BMMH(syms.nindirectsyms));
				indirectsymtbl = new unsigned[BMMH(syms.nindirectsyms)];
				fp.seek(imgoffs + BMMH(syms.indirectsymoff));
				fp.read((char *)indirectsymtbl, BMMH(syms.nindirectsyms) * sizeof(unsigned));

				for (unsigned j = 0; j < BMMH(syms.nindirectsyms); j++) {
					DEBUG_PRINT("%i ", BMMH(indirectsymtbl[j]));
				}

				DEBUG_PRINT("\n");
			}
			break;

		default:
			DEBUG_PRINT("not handled load command %x\n", BMMH(cmd.cmd));
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

	loaded_addr = BMMH(lowest->vmaddr);
	loaded_size = BMMH(highest->vmaddr) - BMMH(lowest->vmaddr) + BMMH(highest->vmsize);

	base = (char *)malloc(loaded_size);

	if (!base) {
		qWarning() << "Cannot allocate memory for copy of image";
		return false;
	}

	for (unsigned i = 0; i < segments.size(); i++) {
		fp.seek(imgoffs + BMMH(segments[i].fileoff));
		ADDRESS  a   = ADDRESS::g(BMMH(segments[i].vmaddr));
		unsigned sz  = BMMH(segments[i].vmsize);
		unsigned fsz = BMMH(segments[i].filesize);
		memset(base + a.m_value - loaded_addr.m_value, 0, sz);
		fp.read(base + a.m_value - loaded_addr.m_value, fsz);
		DEBUG_PRINT("loaded segment %tx %i in mem %i in file\n", a.m_value, sz, fsz);
		QString        name  = QByteArray(segments[i].segname, 17);
		IBinarySection *sect = Image->createSection(name, ADDRESS::n(BMMH(segments[i].vmaddr)),
													ADDRESS::n(BMMH(segments[i].vmaddr) + BMMH(segments[i].vmsize)));
		assert(sect);
		sect->setHostAddr(ADDRESS::g(ADDRESS::value_type(base) + BMMH(segments[i].vmaddr) - loaded_addr.m_value));
		assert((sect->getHostAddr() + sect->getSize()) <= ADDRESS::host_ptr(base + loaded_size));

		unsigned long l = BMMH(segments[i].initprot);
		sect->setBss(false) // TODO
		   .setEndian((machine == Machine::PPC) ? 1 : 0)
		   .setCode(l & VM_PROT_EXECUTE ? 1 : 0)
		   .setData(l & VM_PROT_READ ? 1 : 0)
		   .setReadOnly(~(l & VM_PROT_WRITE) ? 0 : 1);

		for (size_t s_idx = 0; s_idx < sections.size(); s_idx++) {
			if (strcmp(sections[s_idx].segname, segments[i].segname) != 0) {
				continue;
			}

			if ((0 == strcmp(sections[s_idx].sectname, "__cfstring")) ||
				(0 == strcmp(sections[s_idx].sectname, "__cstring"))
				) {
				sect->setAttributeForRange("StringsSection", true,
										   ADDRESS::n(BMMH(sections[s_idx].addr)),
										   ADDRESS::n(BMMH(sections[s_idx].addr) + BMMH(sections[s_idx].size))
										   );
			}

			sect->setAttributeForRange("ReadOnly", (BMMH(sections[i].flags) & VM_PROT_WRITE) ? true : false,
									   ADDRESS::n(BMMH(sections[s_idx].addr)),
									   ADDRESS::n(BMMH(sections[s_idx].addr) + BMMH(sections[s_idx].size))
									   );
		}

		DEBUG_PRINT("loaded segment %tx %i in mem %i in file code=%i data=%i readonly=%i\n", a.m_value, sz, fsz,
					sect->isCode(), sect->isData(), sect->isReadOnly());
	}

	// process stubs_sects
	for (unsigned j = 0; j < stubs_sects.size(); j++) {
		for (unsigned i = 0; i < BMMH(stubs_sects[j].size) / BMMH(stubs_sects[j].reserved2); i++) {
			unsigned startidx = BMMH(stubs_sects[j].reserved1);
			unsigned symbol   = BMMH(indirectsymtbl[startidx + i]);
			ADDRESS  addr     = ADDRESS::g(BMMH(stubs_sects[j].addr) + i * BMMH(stubs_sects[j].reserved2));
			DEBUG_PRINT("stub for %s at %tx\n", strtbl + BMMH(symbols[symbol].n_un.n_strx), addr.m_value);
			char *name = strtbl + BMMH(symbols[symbol].n_un.n_strx);

			if (*name == '_') { // we want printf not _printf
				name++;
			}

			Symbols->create(addr, name).setAttr("Function", true).setAttr("Imported", true);
		}
	}

	// process the remaining symbols
	for (unsigned i = 0; i < symbols.size(); i++) {
		char *name = strtbl + BMMH(symbols[i].n_un.n_strx);

		if ((BMMH(symbols[i].n_un.n_strx) != 0) && (BMMH(symbols[i].n_value) != 0) && (*name != 0)) {
			uint8_t sym_type = symbols[i].n_type;
			bool    is_stab  = (sym_type & N_STAB) != 0;

			if (is_stab) {
				continue; // TODO: handle stab symbols !
			}

			DEBUG_PRINT("symbol %s at %x type %x\n", name, BMMH(symbols[i].n_value), sym_type & N_TYPE);

			if (*name == '_') { // we want main not _main
				name++;
			}

			Symbols->create(ADDRESS::g(BMMH(symbols[i].n_value)), name);
		}
	}

	// process objective-c section
	if (objc_modules != NO_ADDRESS) {
		DEBUG_PRINT("processing objective-c section\n");

		for (unsigned i = 0; i < objc_modules_size; ) {
			struct objc_module *module =
				(struct objc_module *)(ADDRESS::host_ptr(base) + objc_modules - loaded_addr + i).m_value;
			char   *name  = (char *)(intptr_t(base) + BMMH(module->name) - loaded_addr.m_value);
			Symtab symtab = (Symtab)(ADDRESS::host_ptr(base) + BMMH(module->symtab) - loaded_addr).m_value;
#ifdef DEBUG_MACHO_LOADER_OBJC
			fprintf(stdout, "module %s (%i classes)\n", name, BMMHW(symtab->cls_def_cnt));
#endif
			ObjcModule *m = &modules[name];
			m->name = name;

			for (unsigned j = 0; j < BMMHW(symtab->cls_def_cnt); j++) {
				struct objc_class *def   = (struct objc_class *)(base + BMMH(symtab->defs[j]) - loaded_addr.m_value);
				char              *_name = (char *)(ADDRESS::value_type(base) + BMMH(def->name) - loaded_addr.m_value);
#ifdef DEBUG_MACHO_LOADER_OBJC
				fprintf(stdout, "  class %s\n", name);
#endif
				ObjcClass *cl = &m->classes[_name];
				cl->name = _name;
				struct objc_ivar_list *ivars = (struct objc_ivar_list *)(base + BMMH(def->ivars) - loaded_addr.m_value);

				for (unsigned k = 0; k < static_cast<unsigned int>(BMMH(ivars->ivar_count)); k++) {
					struct objc_ivar *ivar  = &ivars->ivar_list[k];
					char             *name2 = (char *)(ADDRESS::value_type(base) + BMMH(ivar->ivar_name) - loaded_addr.m_value);
					char             *types = (char *)(ADDRESS::value_type(base) + BMMH(ivar->ivar_type) - loaded_addr.m_value);
#ifdef DEBUG_MACHO_LOADER_OBJC
					fprintf(stdout, "    ivar %s %s %x\n", name, types, BMMH(ivar->ivar_offset));
#endif
					ObjcIvar *iv = &cl->ivars[name2];
					iv->name   = name2;
					iv->type   = types;
					iv->offset = BMMH(ivar->ivar_offset);
				}

				// this is weird, why is it defined as a ** in the struct but used as a * in otool?
				struct objc_method_list *methods =
					(struct objc_method_list *)(intptr_t(base) + BMMH(def->methodLists) - loaded_addr.m_value);

				for (unsigned k = 0; k < static_cast<unsigned int>(BMMH(methods->method_count)); k++) {
					struct objc_method *method = &methods->method_list[k];
					char               *name3  = (char *)(intptr_t(base) + BMMH(method->method_name) - loaded_addr.m_value);
					char               *types  = (char *)(intptr_t(base) + BMMH(method->method_types) - loaded_addr.m_value);
#ifdef DEBUG_MACHO_LOADER_OBJC
					fprintf(stdout, "    method %s %s %x\n", name, types, BMMH((void *)method->method_imp));
#endif
					ObjcMethod *me = &cl->methods[name3];
					me->name  = name3;
					me->types = types;
					me->addr  = ADDRESS::g(BMMH(method->method_imp));
				}
			}

			i += BMMH(module->size);
		}
	}

	// Give the entry point a symbol
	// ADDRESS entry = GetMainEntryPoint();
	entrypoint = getMainEntryPoint();

	return true;
}


int MachOBinaryLoader::canLoad(QIODevice& dev) const
{
	unsigned char buf[8];

	dev.read((char *)buf, sizeof(buf));

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


bool MachOBinaryLoader::postLoad(void *handle)
{
	Q_UNUSED(handle);
	return false;
}


bool MachOBinaryLoader::displayDetails(const char *fileName, FILE *f
                                       /* = stdout */)
{
	Q_UNUSED(fileName);
	Q_UNUSED(f);
	return false;
}


int MachOBinaryLoader::machORead2(short *ps) const
{
	unsigned char *p = (unsigned char *)ps;
	int           n;

	if (machine == Machine::PPC) {
		n = (int)(p[1] + (p[0] << 8));
	}
	else {
		n = (int)(p[0] + (p[1] << 8));
	}

	return n;
}


int MachOBinaryLoader::machORead4(int *pi) const
{
	short *p = (short *)pi;
	int   n1 = machORead2(p);
	int   n2 = machORead2(p + 1);
	int   n;

	if (machine == Machine::PPC) {
		n = (int)(n2 | (n1 << 16));
	}
	else {
		n = (int)(n1 | (n2 << 16));
	}

	return n;
}


// unsigned int MachOBinaryFile::BMMH(long int & x) {
//    if (swap_bytes) return _BMMH(x); else return x;
// }

int32_t MachOBinaryLoader::BMMH(int32_t x)
{
	if (swap_bytes) {
		return _BMMH(x);
	}
	else {
		return x;
	}
}


uint32_t MachOBinaryLoader::BMMH(uint32_t x)
{
	if (swap_bytes) {
		return _BMMH(x);
	}
	else {
		return x;
	}
}


unsigned short MachOBinaryLoader::BMMHW(unsigned short x)
{
	if (swap_bytes) {
		return _BMMHW(x);
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


ADDRESS MachOBinaryLoader::getImageBase()
{
	return loaded_addr;
}


size_t MachOBinaryLoader::getImageSize()
{
	return loaded_size;
}


DWord MachOBinaryLoader::getDelta()
{
	// Stupid function anyway: delta depends on section
	// This should work for the header only
	//    return (DWord)base - LMMH(m_pPEHeader->Imagebase);
	return (ADDRESS::host_ptr(base) - loaded_addr).m_value;
}


DEFINE_PLUGIN(PluginType::Loader, IFileLoader, MachOBinaryLoader,
			  "DOS4GW loader plugin", "0.4.0", "Boomerang developers")
