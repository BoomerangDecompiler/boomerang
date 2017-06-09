#pragma once

#include "db/statements/statement.h"

/***************************************************************************/ /**
 * TypingStatement is an abstract subclass of Statement. It has a type, representing the type of a reference or an
 * assignment
 ****************************************************************************/
class TypingStatement : public Instruction
{
protected:
	SharedType m_type; ///< The type for this assignment or reference

public:
	TypingStatement(SharedType ty); ///< Constructor

	// Get and set the type.
	SharedType getType() { return m_type; }
	const SharedType& getType() const { return m_type; }
	void setType(SharedType ty) { m_type = ty; }

	virtual bool isTyping() const override { return true; }
};
