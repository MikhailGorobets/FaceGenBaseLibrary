//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgStdString.hpp"
#include "FgString.hpp"

using namespace std;

namespace Fg {

bool
fg64bitOS()
{
#ifdef FG_64
    return true;
#else
    BOOL        is64;
    IsWow64Process(GetCurrentProcess(),&is64);
    return bool(is64);
#endif
}

// Don't warn about deprecation ('GetVersionEx'). TODO: Make os version detection actually work.
#  pragma warning(disable:4996)

string
fgOsName()
{
    OSVERSIONINFOEX     osvi;
    ZeroMemory(&osvi,sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx(LPOSVERSIONINFOW(&osvi));
    string              verStr,
                        unknown = toString(osvi.dwMajorVersion) + "." + toString(osvi.dwMinorVersion);
    if (osvi.wProductType != VER_NT_WORKSTATION)
        unknown += " (server)";
    if (osvi.dwMajorVersion == 6) {
        if (osvi.dwMinorVersion == 0) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "Vista";
            else
                verStr = "Server 2008";
        }
        else if (osvi.dwMinorVersion == 1) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "7";
            else
                verStr = "Server 2008 R2";
        }
        else if (osvi.dwMinorVersion == 2) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "8";
            else
                verStr = "Server 2012";
        }
        else if (osvi.dwMinorVersion == 3) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "8.1";
            else
                verStr = "Server 2012 R2";
        }
        else {
            verStr = unknown;
        }
    }
    else if (osvi.dwMajorVersion == 10) {   // Doesn't work, Win10 appears as Win8. VS2015 libs required to detect.
        if (osvi.dwMinorVersion == 0) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "10";
            else
                verStr = "Server 2016 TP";
        }
        else {
            verStr = unknown;
        }
    }
    else {
        verStr = unknown;
    }
    if (fg64bitOS())
        verStr += " 64bit ";
    else
        verStr += " 32bit ";
    return "Windows " + verStr + Ustring(wstring(osvi.szCSDVersion)).m_str;
}

Ustring
fgComputerName()
{
    TCHAR       buff[512];
    DWORD       len = 512;
    if (GetComputerNameEx(ComputerNamePhysicalNetBIOS,buff,&len))
        return wstring(buff,buff+len);
    if (GetComputerNameEx(ComputerNamePhysicalDnsHostname,buff,&len))
        return wstring(buff,buff+len);
    else
        return wstring(L"Unknown");
}

}

// */
