#ifndef CPPUNIT_EXCEPTION_H
#define CPPUNIT_EXCEPTION_H

#include <exception>
#include <string>

namespace CppUnit {

/*! \brief Exceptions thrown by failed assertions.
 *
 * Exception is an exception that serves
 * descriptive strings through its what() method
 */
class Exception : public std::exception
{
public:

    class Type
    {
    public:
        Type( std::string type ) : m_type ( type ) {}

        bool operator ==( const Type &other ) const
        {
	    return m_type == other.m_type;
        }
    private:
        const std::string m_type;
    };


    Exception( std::string  message = "", 
	       long lineNumber = UNKNOWNLINENUMBER, 
	       std::string fileName = UNKNOWNFILENAME);
    Exception (const Exception& other);

    virtual ~Exception () throw();

    Exception& operator= (const Exception& other);

    const char *what() const throw ();

    long lineNumber ();
    std::string fileName ();

    static const std::string UNKNOWNFILENAME;
    static const long UNKNOWNLINENUMBER;

    virtual Exception *clone() const;
    
    virtual bool isInstanceOf( const Type &type ) const;

    static Type type();

private:
    // VC++ does not recognize call to parent class when prefixed
    // with a namespace. This is a workaround.
    typedef std::exception SuperClass;

    std::string m_message;
    long m_lineNumber;
    std::string m_fileName;
};


} // namespace CppUnit

#endif // CPPUNIT_EXCEPTION_H

