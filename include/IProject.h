#ifndef IPROJECT
#define IPROJECT

class IBinaryImage;
class QByteArray;
struct ITypeRecovery;
/**
 * @brief The Project interface class
 */
class IProject {
public:
    virtual ~IProject() {}
    virtual QByteArray &filedata() = 0;
    virtual IBinaryImage *image() = 0;
    virtual void typeEngine(ITypeRecovery *e) = 0;
    virtual ITypeRecovery *typeEngine() = 0;
};

#endif // IPROJECT

