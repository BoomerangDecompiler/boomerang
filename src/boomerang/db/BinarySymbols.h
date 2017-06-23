#pragma once

// TODO this is unused and has a name clash with struct BinarySymbol in SymTab.h
#if 0
#include "db/IBinarySymbols.h"
#include <QMap>
#include <QVariantMap>
class BinarySymbols;
class BinarySymbol : public IBinarySymbol
{
protected:
	BinarySymbols *Parent;
	QString name;
	ADDRESS location;
	SharedType type;
	QVariantMap Attributes;

public:
	BinarySymbol(BinarySymbols *p)
		: Parent(p)
	{
	}

	// IBinarySymbol interface

public:
	IBinarySymbol& setName(const QString& name) override;
	IBinarySymbol& setAttr(const QString& name, const QVariant&)  override;
	IBinarySymbol& setSize(size_t sz) override;
};

class BinarySymbols : public IBinarySymbolTable
{
	friend class BinarySymbol;

public:
	BinarySymbols();

public:
	virtual IBinarySymbol& addSymbol(ADDRESS a);
	virtual bool hasSymbolAt(ADDRESS a);
	virtual bool hasSymbol(const QString& name);
	virtual void addEntryPointSymbol(const QString&);

	virtual void addEntryPoint(ADDRESS);
	virtual void removeEntryPoint(ADDRESS);
	void addImport(ADDRESS) override;
	virtual void addExport(ADDRESS);

private:
	std::vector<BinarySymbol *> m_symbols;
	QMap<QString, BinarySymbol *> m_nameToSymbolMap;
	QMap<ADDRESS, BinarySymbol *> m_addressToSymbolMap;
};

#endif
