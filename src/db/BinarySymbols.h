#pragma once

#include "IBinarySymbols.h"
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
	// IBinarySymbols interface

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
	std::vector<BinarySymbol *> Symbols;
	QMap<QString, BinarySymbol *> NameToSymbolMap;
	QMap<ADDRESS, BinarySymbol *> AddressToSymbolMap;
};

#endif // BINARYSYMBOLS_H
