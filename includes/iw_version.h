// --------------------------------------------------------------------------
///
/// @file iw_version.h
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_VERSION_H_
#define _IW_VERSION_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_util.h"

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// The major version number of the InstaWorks library.
#define IW_VERSION_MAJOR        0

/// The minor version number of the InstaWorks library.
#define IW_VERSION_MINOR        20

/// A string representation of the version number.
#define IW_VER_STR              IW_STR(IW_VERSION_MAJOR)"."IW_STR(IW_VERSION_MINOR)

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_VERSION_H_

// --------------------------------------------------------------------------
