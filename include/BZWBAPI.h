/* BZWorkbench
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZWB_API_H_
#define BZWB_API_H_

#ifdef _WIN32
#ifdef INSIDE_BZWB
#define BZWB_API __declspec( dllexport )
#else
#define BZWB_API __declspec( dllimport )
#endif
#define BZWB_PLUGIN_CALL
#ifndef strcasecmp
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define strcasecmp _stricmp
#else
#define strcasecmp stricmp
#endif
#endif
#else
#define BZEB_API
#define BZEB_PLUGIN_CALL extern "C"
#endif

#define BZWB_API_VERSION	1

#define BZWB_GET_PLUGIN_VERSION BZF_PLUGIN_CALL int bzwb_GetVersion ( void ) { return BZWB_API_VERSION;}


#endif /*BZWB_API_H_*/