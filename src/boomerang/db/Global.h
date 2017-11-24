#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/util/Address.h"
#include "boomerang/util/Util.h"
#include "boomerang/type/type/Type.h"

class Prog;

/**
 * A global variable in the decompiled program.
 */
class Global : public Printable
{
public:
    Global(SharedType type, Address addr, const QString& name, Prog *prog);
    Global(const Global& other) = delete;
    Global(Global&& other) = default;

    virtual ~Global() override = default;

    Global& operator=(const Global& other) = delete;
    Global& operator=(Global&& other) = delete;

public:
    /// @copydoc Printable::toString
    QString toString() const override;

    SharedType getType() const { return m_type; }
    void setType(SharedType ty) { m_type = ty; }
    void meetType(SharedType ty);

    Address getAddress()     const { return m_addr; }
    const QString& getName() const { return m_name; }

    /// return true if \p address is contained within this global.
    bool containsAddress(Address addr) const;

    /// Get the initial value as an expression (or nullptr if not initialised)
    SharedExp getInitialValue(const Prog *prog) const;

private:
    SharedType m_type;
    Address m_addr;
    QString m_name;
    Prog *m_program;
};
