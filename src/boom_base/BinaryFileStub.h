#pragma once

#include "boom_base/BinaryFile.h"


class FileLoaderStub : public IFileLoader
{
public:
	FileLoaderStub();
	virtual ~FileLoaderStub() {}

	void unload() override {}              ///< Unload the image
	bool getNextMember() { return false; } ///< Load next member of archive
	bool open(const char */*sName*/) { return false; } ///< Open for r/w; pv
	void close() override {}               ///< Close file opened with Open()
	LoadFmt getFormat() const override;   ///< Get format (e.g. LOADFMT_ELF)
	Machine getMachine() const override;   ///< Get machine (e.g. MACHINE_SPARC)

	QString getFilename() const { return m_fileName; }
	bool isLibrary() const;
	ADDRESS getImageBase() override;
	size_t getImageSize() override;

	// Header functions
	virtual ADDRESS getFirstHeaderAddress(); // Get ADDRESS of main header

	//
	//    --    --    --    --    --    --    --    --    --    --    --
	//
	// Internal information
	// Dump headers, etc
	bool displayDetails(const char *fileName, FILE *f = stdout) override;

	// Analysis functions
	virtual std::list<SectionInfo *>& getEntryPoints(const char *pEntry = "main");
	ADDRESS getMainEntryPoint() override;
	ADDRESS getEntryPoint() override;

	// Get a map from ADDRESS to const char*. This map contains the native
	// addresses and symbolic names of global data items (if any) which are
	// shared with dynamically linked libraries. Example: __iob (basis for
	// stdout).The ADDRESS is the native address of a pointer to the real
	// dynamic data object.
	// The caller should delete the returned map.
	virtual std::map<ADDRESS, const char *> *getDynamicGlobalMap();

	// Not meant to be used externally, but sometimes you just
	// have to have it.
	char *getStrPtr(int idx, int offset); // Calc string pointer

	// Similarly here; sometimes you just need to change a section's
	// link and info fields
	// idx is the section index; link and info are indices to other
	// sections that will be idx's sh_link and sh_info respectively
	void setLinkAndInfo(int idx, int link, int info);

protected:
	bool loadFromMemory(QByteArray& /*data*/) override { return false; }
	bool postLoad(void *handle) override;         // Called after loading archive member

private:
	const char *m_fileName; // Pointer to input file name
};
