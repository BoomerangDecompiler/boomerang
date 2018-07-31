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
#  ifdef _WIN32
#     if defined(BOOMERANG_BUILD_SHARED) /* build dll */
#         define BOOMERANG_API __declspec(dllexport)
#     elif !defined(BOOMERANG_BUILD_STATIC) /* use dll */
#         define BOOMERANG_API __declspec(dllimport)
#     else /* static library */
#         define BOOMERANG_API
#     endif
#  else
#    define BOOMERANG_API __attribute__((visibility("default")))
#  endif
#endif
