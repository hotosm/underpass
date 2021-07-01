//
// Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef DSODEFS_H
#define DSODEFS_H

#ifdef HAVE_CONFIG_H
//#include "uconfig.h"
#endif

#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32)
// #ifdef BUILDING_DLL
#ifdef DLL_EXPORT
#define DSOEXPORT __declspec(dllexport)
#else
// Temporarily commented because of VC++ compiler problems 
#define DSOEXPORT // __declspec(dllimport)
#endif

#define DSOLOCAL
#elif defined(__OS2__)
#ifdef BUILDING_DLL
#define DSOEXPORT __declspec(dllexport)
#else
// Temporarily commented because of VC++ compiler problems 
#define DSOEXPORT // __declspec(dllimport)
#endif

#define DSOLOCAL

#else
#ifdef HAVE_GNUC_VISIBILITY
#define DSOEXPORT __attribute__ ((visibility("default")))
#define DSOLOCAL __attribute__ ((visibility("hidden")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550) /* Sun Studio >= 8 */
#define DSOEXPORT __global
#define DSOLOCAL __hidden
#else
#define DSOEXPORT
#define DSOLOCAL
#endif
#endif

#ifdef USE_TESTSUITE
# define DSOTEXPORT DSOEXPORT
#else
# define DSOTEXPORT 
#endif

#endif // DSODEFS_H
