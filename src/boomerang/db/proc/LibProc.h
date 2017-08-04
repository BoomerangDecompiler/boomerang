#pragma once

#include "Proc.h"


/***************************************************************************/ /**
 * LibProc class.
 ******************************************************************************/
class LibProc : public Function
{
public:

    /***************************************************************************/ /**
     * \brief        Constructor with name, native address.
     * \param        mod - Module that contains this Function
     * \param        name - Name of procedure
     * \param        address - Native address of entry point of procedure
     ******************************************************************************/
    LibProc(Module *mod, const QString& name, Address address);
    virtual ~LibProc() = default;

    QString toString() const override;

    /// Return true, since is a library proc
    bool isLib() const override { return true; }
    virtual bool isNoReturn() const override;
    virtual SharedExp getProven(SharedExp left) override;

    virtual SharedExp getPremised(SharedExp /*left*/) override { return nullptr; } ///< Get the RHS that is premised for left
    virtual bool isPreserved(SharedExp e) override;                            ///< Return whether e is preserved by this proc
    void getInternalStatements(StatementList& internal);
};

