#ifndef __INTELCOFFFILE_H__
#define __INTELCOFFFILE_H__

#include "BinaryFile.h"
#include "SymTab.h"

#define PACKED __attribute__((packed))

struct coff_header
{
        ushort  coff_magic;
        ushort  coff_sections;
        ulong   coff_timestamp;
        ulong   coff_symtab_ofs;
        ulong   coff_num_syment;
        ushort  coff_opthead_size;
        ushort  coff_flags;
} PACKED;

class IntelCoffFile : public BinaryFile
{
public:
	//
	// Interface
	//
	IntelCoffFile();
	~IntelCoffFile();

	virtual void UnLoad();

	virtual bool Open(const char *sName);
	virtual bool RealLoad(const char*);
	virtual bool PostLoad(void*);
	virtual void Close();

	virtual LOAD_FMT    GetFormat() const;
	virtual MACHINE     GetMachine() const;
	virtual const char *getFilename() const;
	virtual bool        isLibrary() const;
	virtual std::list<const char *> getDependencyList();
	virtual ADDRESS     getImageBase();
	virtual size_t      getImageSize();
	virtual ADDRESS     GetMainEntryPoint();
	virtual ADDRESS     GetEntryPoint();
	virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");

	virtual const char* SymbolByAddress(ADDRESS uNative);
        // Lookup the name, return the address. If not found, return NO_ADDRESS
	// virtual ADDRESS         GetAddressByName(const char* pName, bool bNoTypeOK = false);
	// virtual void            AddSymbol(ADDRESS uNative, const char *pName) { }
	virtual bool IsDynamicLinkedProc(ADDRESS uNative);
	virtual bool IsRelocationAt(ADDRESS uNative);
	virtual std::map<ADDRESS, std::string> &getSymbols();

	virtual int readNative4(ADDRESS a);
	virtual int readNative2(ADDRESS a);
	virtual int readNative1(ADDRESS a);

private:
	//
	// Internal stuff
	//
	const char *m_pFilename;
	int m_fd;
	std::list<SectionInfo*> m_EntryPoints;
	std::list<ADDRESS> m_Relocations;
	struct coff_header m_Header;

	PSectionInfo AddSection(SectionInfo*);
	unsigned char* getAddrPtr(ADDRESS a, ADDRESS range);
	int readNative(ADDRESS a, unsigned short n);

	SymTab m_Symbols;
};

#endif	// !defined(__INTELCOFFFILE_H__)
