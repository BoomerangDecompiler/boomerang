#include "boom_base/BinaryFile.h"

class BinaryFileStub : public LoaderInterface
{
public:
	BinaryFileStub();                      // Constructor
	virtual ~BinaryFileStub() {}
	void UnLoad() override {}              // Unload the image
	bool GetNextMember() { return false; } // Load next member of archive
	bool Open(const char */*sName*/) { return false; } // Open for r/w; pv
	void Close() override {}               // Close file opened with Open()
	LOAD_FMT GetFormat() const override;   // Get format (e.g. LOADFMT_ELF)
	MACHINE getMachine() const override;   // Get machine (e.g. MACHINE_SPARC)

	QString getFilename() const { return m_pFileName; }
	bool isLibrary() const;
	ADDRESS getImageBase() override;
	size_t getImageSize() override;

	// Header functions
	virtual ADDRESS GetFirstHeaderAddress(); // Get ADDRESS of main header

	//
	//    --    --    --    --    --    --    --    --    --    --    --
	//
	// Internal information
	// Dump headers, etc
	bool DisplayDetails(const char *fileName, FILE *f = stdout) override;

	// Analysis functions
	virtual std::list<SectionInfo *>& GetEntryPoints(const char *pEntry = "main");
	ADDRESS GetMainEntryPoint() override;
	ADDRESS GetEntryPoint() override;

	// Get a map from ADDRESS to const char*. This map contains the native
	// addresses and symbolic names of global data items (if any) which are
	// shared with dynamically linked libraries. Example: __iob (basis for
	// stdout).The ADDRESS is the native address of a pointer to the real
	// dynamic data object.
	// The caller should delete the returned map.
	virtual std::map<ADDRESS, const char *> *GetDynamicGlobalMap();

	// Not meant to be used externally, but sometimes you just
	// have to have it.
	char *GetStrPtr(int idx, int offset); // Calc string pointer

	// Similarly here; sometimes you just need to change a section's
	// link and info fields
	// idx is the section index; link and info are indices to other
	// sections that will be idx's sh_link and sh_info respectively
	void SetLinkAndInfo(int idx, int link, int info);

	const char *m_pFileName; // Pointer to input file name

protected:
	bool LoadFromMemory(QByteArray& /*data*/) override { return false; }
	bool PostLoad(void *handle) override;         // Called after loading archive member

private:
};
