#ifndef IPROJECT
#define IPROJECT

class IBinaryImage;
class QByteArray;
/**
 * @brief The Project interface class
 */
class IProject {
public:
    virtual QByteArray &filedata() = 0;
    virtual IBinaryImage *image() = 0;
};

#endif // IPROJECT

