#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Boomerang.h"


#include "boomerang/codegen/CCodeGenerator.h"
#include "boomerang/core/Project.h"
#include "boomerang/core/Watcher.h"
#include "boomerang/db/CFGCompressor.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/frontend/Frontend.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/CFGDotWriter.h"

#include <ctime>


static Boomerang *g_boomerang;


Boomerang::Boomerang()
{
}


Boomerang *Boomerang::get()
{
    if (!g_boomerang) {
        g_boomerang = new Boomerang();
    }

    return g_boomerang;
}


void Boomerang::destroy()
{
    delete g_boomerang;
    g_boomerang = nullptr;
}
