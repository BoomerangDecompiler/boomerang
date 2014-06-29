#ifndef BINARYIMAGE_H
#define BINARYIMAGE_H
#include "SectionInfo.h"
#include "IBinaryImage.h"

#include <boost/icl/interval_map.hpp>

struct SectionHolder {
    SectionHolder() : val(nullptr) {}
    SectionHolder(SectionInfo *inf) : val(inf) {}

    SectionInfo *operator->() { return val;}
    SectionInfo &operator*() const { return *val;}
    operator SectionInfo *() { return val;}
    operator const SectionInfo *() const { return val;}
    SectionInfo *val;
    SectionHolder operator+=(const SectionHolder &/*s*/) {
        throw std::runtime_error("Cannot aggregate SectionInfos !");
    }
};
struct BinaryImage : public IBinaryImage {
protected:
    BinaryImage(const BinaryImage &); // prevent copy-construction
    BinaryImage &operator=(const BinaryImage &); // prevent assignment
public:
    /// The type for the list of functions.
    typedef boost::icl::interval_map<ADDRESS,SectionHolder> MapAddressRangeToSection;

public:
    BinaryImage();
    ~BinaryImage();
    // IBinaryImage interface
    void reset() override;

    size_t GetNumSections() const override { return Sections.size(); }
    ADDRESS imageToSource(ADDRESS); //! convert image address ( host pointer into image data ) to valid Source machine ADDRESS
    ADDRESS sourceToImage(ADDRESS); //! convert Source machine ADDRESS into valid image ADDRESS

    char readNative1(ADDRESS nat) override;
    int readNative2(ADDRESS nat) override;
    int readNative4(ADDRESS nat) override;
    QWord readNative8(ADDRESS nat) override;
    float readNativeFloat4(ADDRESS nat) override;
    double readNativeFloat8(ADDRESS nat) override;
    void writeNative4(ADDRESS nat, uint32_t n) override;
    void calculateTextLimits();
    //! Find the section, given an address in the section
    const SectionInfo *getSectionInfoByAddr(ADDRESS uEntry) const;

    int GetSectionIndexByName(const QString &sName) override;
    SectionInfo *GetSectionInfoByName(const QString &sName) override;
    const SectionInfo *GetSectionInfo(int idx) const override { return Sections[idx]; }
    bool isReadOnly(ADDRESS uEntry) override;
    ADDRESS getLimitTextLow() override;
    ADDRESS getLimitTextHigh() override;
    ptrdiff_t getTextDelta() override { return TextDelta; }

    SectionInfo *createSection(const QString &name, ADDRESS from, ADDRESS to);

    iterator                begin()       { return Sections.begin(); }
    const_iterator          begin() const { return Sections.begin(); }
    iterator                end  ()       { return Sections.end();   }
    const_iterator          end  () const { return Sections.end();   }
    size_t                  size()  const { return Sections.size(); }
    bool                    empty() const { return Sections.empty(); }

private:
    ADDRESS limitTextLow;
    ADDRESS limitTextHigh;
    ptrdiff_t TextDelta;
    MapAddressRangeToSection SectionMap;
    SectionListType Sections; //!< The section info


};


#endif // BINARYIMAGE_H
