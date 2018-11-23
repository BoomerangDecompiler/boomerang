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


#include "boomerang/ifc/ICodeGenerator.h"
#include "boomerang/ifc/IDecoder.h"
#include "boomerang/ifc/IFileLoader.h"
#include "boomerang/ifc/IFrontEnd.h"
#include "boomerang/ifc/ILogSink.h"
#include "boomerang/ifc/ISymbolProvider.h"
#include "boomerang/ifc/ITypeRecovery.h"


enum class PluginType
{
    Invalid = 0,
    CodeGenerator,
    Decoder,
    FileLoader,
    FrontEnd,
    SymbolProvider,
    TypeRecovery
};

template<PluginType pty>
struct PluginIfcTraits;

#define DECLARE_PLUGIN_IFC_TRAIT(pty, the_ifc)                                                     \
    template<>                                                                                     \
    struct PluginIfcTraits<pty>                                                                    \
    {                                                                                              \
        typedef the_ifc IFC;                                                                       \
    }

DECLARE_PLUGIN_IFC_TRAIT(PluginType::CodeGenerator, ICodeGenerator);
DECLARE_PLUGIN_IFC_TRAIT(PluginType::Decoder, IDecoder);
DECLARE_PLUGIN_IFC_TRAIT(PluginType::FileLoader, IFileLoader);
DECLARE_PLUGIN_IFC_TRAIT(PluginType::FrontEnd, IFrontEnd);
DECLARE_PLUGIN_IFC_TRAIT(PluginType::SymbolProvider, ISymbolProvider);
DECLARE_PLUGIN_IFC_TRAIT(PluginType::TypeRecovery, ITypeRecovery);
