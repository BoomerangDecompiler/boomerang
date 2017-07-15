#pragma once

/*
 * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file        register.h
 * OVERVIEW:    Header information for the Register class.
 ******************************************************************************/

#include <string>
#include <stdint.h>
#include <memory>
#include <QString>

class Type;
typedef std::shared_ptr<Type> SharedType;

/***************************************************************************/ /**
 * \class  Register
 * Summarises one line of the \@REGISTERS section of an SSL
 * file. This class is used extensively in sslparser.y, and there is a public
 * member of RTLInstDict called DetRegMap which gives a Register object from
 * a register index (register indices may not always be sequential, hence it's
 * not just an array of Register objects).
 * This class plays a more active role in the Interpreter, which is not yet
 * integrated into uqbt
 ******************************************************************************/
class Register
{
public:
	Register(); ///< needed for use in stl classes.
	Register(const Register&);

	/***************************************************************************/ /**
	 * \brief      Copy operator
	 * \param      r2 - Reference to another Register object (to be copied)
	 * \returns    This object
	 ******************************************************************************/
	Register& operator=(const Register& r2);

	/***************************************************************************/ /**
	 * \brief   Equality operator
	 * \param   r2 - Reference to another Register object
	 * \returns True if the same
	 ******************************************************************************/
	bool operator==(const Register& r2) const;

	/***************************************************************************/ /**
	 * \brief   Comparison operator (to establish an ordering)
	 * \param   r2 - Reference to another Register object
	 * \returns true if this name is less than the given Register's name
	 ******************************************************************************/
	bool operator<(const Register& r2) const;

	///////////////////////////////////////////////////////////////////////////////
	// access and set functions
	///////////////////////////////////////////////////////////////////////////////

	/***************************************************************************/ /**
	 * \brief      Set the name for this register
	 * \param      name - name to set it to
	 ******************************************************************************/
	void setName(const QString& name);

	void setSize(uint16_t s) { m_size = s; }
	void setFloat(bool f) { m_fltRegister = f; }
	void setAddress(void *p) { m_address = p; }

	/* These are only used in the interpreter */

	/***************************************************************************/ /**
	 * \brief    Get the name for this register
	 * \returns  The name as a character string
	 ******************************************************************************/
	const QString& getName() const;

	void *getAddress() const { return m_address; }

	int getSize() const { return m_size; }

	/***************************************************************************/ /**
	 * \brief   Get the type for this register
	 * \returns The type as a pointer to a Type object
	 ******************************************************************************/
	SharedType getType() const;

	/**
	 * Set the mapped index. For COVERS registers, this is the lower register
	 * of the set that this register covers. For example, if the current register
	 * is f28to31, i would be the index for register f28
	 * For SHARES registers, this is the "parent" register, e.g. if the current
	 * register is %al, the parent is %ax (note: not %eax)
	 */
	void setMappedIndex(int i) { m_mappedIndex = i; }

	/** Set the mapped offset. This is the bit number where this register starts,
	 * e.g. for register %ah, this is 8. For COVERS regisers, this is 0 */
	void setMappedOffset(int i) { m_mappedOffset = i; }

	/// Get the mapped index (see above)
	int getMappedIndex() const { return m_mappedIndex; }

	/// Get the mapped offset (see above)
	int getMappedOffset() const { return m_mappedOffset; }

	/// \returns true if this is a floating point register
	bool isFloat() const { return m_fltRegister; }

private:
	QString m_name;
	uint16_t m_size;
	void *m_address;
	int m_mappedIndex;
	int m_mappedOffset;
	bool m_fltRegister; ///< True if this is a floating point register
};
