//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 19, 2011
//

#include "stdafx.h"
#include "FgPlatform.hpp"

int
fgSizeofPtr()
{return sizeof(void*); }

bool
fgIs64bit()
{
#ifdef FG_64
    return true;
#else
    return false;
#endif
}

bool
fgIsDebug()
{
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}

std::string
fgBitsString()
{
#ifdef FG_64
    return "64";
#else
    return "32";
#endif
}
