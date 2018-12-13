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


#ifndef BOOMERANG_API
#    ifdef _WIN32
#        if defined(BOOMERANG_BUILD_SHARED) /* build dll */
#            define BOOMERANG_API __declspec(dllexport)
#        else /* use dll */
#            define BOOMERANG_API __declspec(dllimport)
#        endif
#    else
#        define BOOMERANG_API __attribute__((visibility("default")))
#    endif
#endif

#ifndef BOOMERANG_PLUGIN_API
#    ifdef _WIN32
#        if defined(BOOMERANG_BUILD_PLUGIN) /* build plugin dll */
#            define BOOMERANG_PLUGIN_API __declspec(dllexport)
#        else /* use dll */
#            define BOOMERANG_PLUGIN_API __declspec(dllimport)
#        endif
#    else
#        define BOOMERANG_PLUGIN_API __attribute__((visibility("default")))
#    endif
#endif
