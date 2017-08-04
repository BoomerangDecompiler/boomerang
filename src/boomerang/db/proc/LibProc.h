#pragma once

#include "Proc.h"


/**
 * Class for library procedures called by the program (like printf).
 */
class LibProc : public Function
{
public:
    /***************************************************************************/ /**
     * \brief        Constructor with name, native address.
     * \param        address - Native address of entry point of procedure
     * \param        name - Name of procedure
     * \param        module - Module that contains this Function
     ******************************************************************************/
    LibProc(Address address, const QString& name, Module *module);
    virtual ~LibProc() = default;

    QString toString() const override;

    /// \copydoc Function::isLib
    bool isLib() const override { return true; }

    /// \copydoc Function::isNoReturn
    virtual bool isNoReturn() const override;

    /// \copydoc Function::getProven
    virtual SharedExp getProven(SharedExp left) override;

    /// \copydoc Function::getPremised
    /// Get the RHS that is premised for left
    virtual SharedExp getPremised(SharedExp) override { return nullptr; }

    /// \copydoc Function::isPreserved
    /// Return whether \p e is preserved by this proc
    virtual bool isPreserved(SharedExp e) override;
};

