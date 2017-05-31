#include "BinarySymbols.h"

#include <QDebug>
#include <cassert>
BinarySymbols::BinarySymbols()
{
}


/// mark address as containing pointer to imported function
/// the function name and possibly type should be contained in binarysymbols table
void BinarySymbols::addImport(ADDRESS addr)
{
	assert(false);
}


IBinarySymbol& BinarySymbols::addSymbol(ADDRESS a)
{
	if (AddressToSymbolMap.contains(a)) {
		qDebug() << "Attempt to insert a symbol twice";
		return *AddressToSymbolMap[a];
	}

	Symbols.push_back(new BinarySymbol(this));
	AddressToSymbolMap[a] = Symbols.back();
	return *Symbols.back();
}


bool BinarySymbols::hasSymbolAt(ADDRESS a)
{
	return AddressToSymbolMap.contains(a);
}


bool BinarySymbols::hasSymbol(const QString& name)
{
	return NameToSymbolMap.contains(name);
}


void BinarySymbols::addEntryPointSymbol(const QString&)
{
	assert(false);
}


void BinarySymbols::addEntryPoint(ADDRESS)
{
	assert(false);
}


void BinarySymbols::removeEntryPoint(ADDRESS)
{
	assert(false);
}


void BinarySymbols::addExport(ADDRESS)
{
	assert(false);
}


IBinarySymbol& BinarySymbol::setName(const QString& name)
{
	assert(false);

	if (Parent->hasSymbol(name)) {
	}

	return *this;
}


IBinarySymbol& BinarySymbol::setAttr(const QString& name, const QVariant&)
{
	assert(false);
	return *this;
}


IBinarySymbol& BinarySymbol::setSize(size_t sz)
{
	assert(false);
	return *this;
}
