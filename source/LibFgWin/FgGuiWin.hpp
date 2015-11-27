//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 18, 2011
//
// FgGuiWin objects may or may not be win32 windows. This is important for avoiding
// excessive depth in the tree as win64 can choke on as little as 7 levels. Due to this,
// FgGuiWin objects call a virtual function to create, move or hide/show children, which
// keep track of their own win32 handles.
//
// To avoid duplication, all windows sizing is done through 'moveWindow' which means that
// all windows are created with size 0 (during 'create') which means that all windows receive
// an initial size 0 WM_SIZE message which they must ignore or the initial (startup) display
// can be blank.
//
// Possible todo: switch to idiom of creating all child windows without visible flag, then
// calling ShowWindow after creation. Might avoid all the WM_SIZE(0,0) messages.
//

#ifndef FGGUIWIN_HPP
#define FGGUIWIN_HPP

#include "FgStdLibs.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgOpt.hpp"

// Declared in LibFgBase but defined differently in each OS-specific library:
struct  FgGuiOsBase
{
    virtual
    ~FgGuiOsBase()
    {};

    // Recursively create the win32 window objects using the idiomatic approach of
    // recursing the creation call within the WM_CREATE handler. This function
    // is only called once:
    virtual void
    create(
        HWND            hwndParent,
        int             ident,          // WinImpl window index
        const FgString & store,         // Root filename for storing state.
        DWORD           extStyle=NULL,
        bool            visible=true)   // Visible on creation ?
        = 0;

    // Calls DestroyWindow. Used for dynamic windows - don't want this in destructor.
    virtual void
    destroy() = 0;

    // Minimum size must be constant for an object - it cannot depend on dynamic content,
    // since otherwise updating content could force a resize of the entire window.
    // So for windows that contain dynamic sub-windows, we have a problem ...
    // Minimum sizes do not include any padding. It is the responsibility of the
    // parent container window to provide padding between elements:
    virtual FgVect2UI
    getMinSize() const = 0;

    // Do the window contents BENEFIT from stretching in the given dimensions ?
    // Any window size can always be expanded but the contents do not necessarily expand
    // to fill, or even if they do it may not improve anything (eg. a button) so this
    // propertly allows space to be allocated somewhere more beneficial (if possible):
    virtual FgVect2B
    wantStretch() const = 0;

    virtual void
    updateIfChanged() = 0;

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz) = 0;

    // Since Windows automatically makes all sub-windows of a hidden window hidden, we keep
    // sub-windows in a visible or hidden state as if the parent window was visible, to
    // minimize state changes and tracking:
    virtual void
    showWindow(bool) = 0;

    // We sometimes need to pass windows messages on to sub-windows (eg. WM_MOUSEWHEEL) so
    // we need to query their handle. TODO: This approach doesn't work unless we also pass
    // down the relative position of the cursor so that split windows know which handle to
    // return !
    virtual
    FgOpt<HWND>
    getHwnd()
    {return FgOpt<HWND>(); }

    virtual void
    saveState()
    {}
};

struct  FgGuiWinStatics
{
    HINSTANCE       hinst;
    HWND            hwndMain;

    FgGuiWinStatics() : hinst(0), hwndMain(0) {}
};

extern FgGuiWinStatics s_fgGuiWin;

LRESULT
fgWinCallCatch(boost::function<LRESULT(void)> func,const string & className);

template<class WinImpl>
LRESULT CALLBACK
fgStatWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    static string   className = typeid(WinImpl).name();
    if (message == WM_NCCREATE) {
        // The "this" pointer passed to CreateWindowEx is returned here in 'lParam'.
        // Save to the Windows instance user data for retrieval in later calls:
        SetWindowLongPtr(hwnd,0,(LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
    }
    // Get object pointer from hwnd:
    WinImpl * wnd = (WinImpl *)GetWindowLongPtr(hwnd,0);
    if (wnd == 0)   // For before WM_NCCREATE
        return DefWindowProc(hwnd,message,wParam,lParam);
    else
        return fgWinCallCatch(boost::bind(&WinImpl::wndProc,wnd,hwnd,message,wParam,lParam),className);
}

struct  FgCreateChild
{
    DWORD       style;
    DWORD       extStyle;
    bool        useFillBrush;
    HCURSOR     cursor;
    // visible on creation ? Note that WS_VISIBLE is recursely applied in the negative case;
    // if a parent window is not visible, it's children are not visible:
    bool        visible;

    FgCreateChild() :
        style(NULL),
        extStyle(NULL),
        useFillBrush(true),
        cursor(LoadCursor(NULL,IDC_ARROW)),
        visible(true)
    {}
};

template<class ChildImpl>
HWND
fgCreateChild(
    HWND            parentHwnd,
    // WinImpl window identifier. Must be unique per instantiation for given parent if messages from
    // child to parent need to be processed:
    int             ident,
    // Passed back in windows callback function and used to map to object's wndProc:
    ChildImpl *     thisPtr,
    FgCreateChild   opt)
{
    std::string     classNameA = typeid(ChildImpl).name();
    // Different class options mean different classes:
    classNameA +=   fgToString(uint(opt.cursor)) + "_" +
                    fgToString(uint(opt.useFillBrush));
    std::wstring    className = FgString(classNameA).as_wstring();
    FGASSERT(className.length() < 256);     // Windows limit
    WNDCLASSEX  wcl;
    wcl.cbSize = sizeof(wcl);
    if (GetClassInfoEx(s_fgGuiWin.hinst,className.c_str(),&wcl) == 0)
    {
        wcl.style = CS_HREDRAW | CS_VREDRAW;
        wcl.lpfnWndProc = &fgStatWndProc<ChildImpl>;
        wcl.cbClsExtra = 0;
        wcl.cbWndExtra = sizeof(void *);
        wcl.hInstance = s_fgGuiWin.hinst;
        wcl.hIcon = NULL;
        wcl.hCursor = opt.cursor;
        // COLOR_BTNFACE matches background color used by windows for buttons & controls [Petzold 374].
        // Set to NULL in non-leaf windows to minimize flicker caused by erasing then redrawing.
        wcl.hbrBackground = opt.useFillBrush ? GetSysColorBrush(COLOR_BTNFACE) : NULL,
        wcl.lpszMenuName = NULL;
        wcl.lpszClassName = className.c_str();
        wcl.hIconSm = NULL;
        FGASSERTWIN(RegisterClassEx(&wcl));
    }
    // CreateWindowEx sends WM_CREATE and certain other messages before returning.
    // This is done so that the caller can send messages to the child window immediately
    // after calling this function.
    int     flags = WS_CHILD | opt.style;   // WS_CHILD == WS_CHILDWINDOW
    // Parent must be visible for child to be visible. Idiomatic approach is to create without
    // visible flag, then call ShowWindow. If created with visible flag, windows sends a WM_SHOWWINDOW:
    if (opt.visible) flags = flags | WS_VISIBLE;
    HWND hwnd =
        CreateWindowEx(
            opt.extStyle,
            className.c_str(), 
            NULL,
            flags,
            // Zeros for position and size are the idiomatic way to create a child window, which
            // is strange because Windows sends a WM_SIZE message of zeros after creation, perhaps
            // I should be creating not visible then making visible after ?
            0,0,0,0,
            parentHwnd,
            (HMENU)ident,
            s_fgGuiWin.hinst,   // Contrary to MSDN docs, this is used on all WinOSes to
                                // disambiguate the class name over different modules [Raymond Chen]
            thisPtr);           // Value to be sent as argument with WM_NCCREATE message
    FGASSERTWIN(hwnd != 0);
    return hwnd;
}

template<class WinImpl>
HWND
fgCreateDialog(
    const FgString &    title,
    HWND                ownerHwnd,
    WinImpl *           thisPtr)
{
    std::wstring    className = FgString("FgDialogClass").as_wstring();
    WNDCLASSEX      wcl;
    wcl.cbSize = sizeof(wcl);
    if (GetClassInfoEx(s_fgGuiWin.hinst,className.c_str(),&wcl) == 0) {
        wcl.style = NULL;
        wcl.lpfnWndProc = &fgStatWndProc<WinImpl>;
        wcl.cbClsExtra = 0;
        wcl.cbWndExtra = sizeof(void *);
        wcl.hInstance = s_fgGuiWin.hinst;
        wcl.hIcon = NULL;
        wcl.hCursor = LoadCursor(NULL,IDC_ARROW);
        wcl.hbrBackground = GetSysColorBrush(COLOR_BTNFACE),
        wcl.lpszMenuName = NULL;
        wcl.lpszClassName = className.c_str();
        wcl.hIconSm = NULL;
        FGASSERTWIN(RegisterClassEx(&wcl));
    }
    RECT            rect;
    GetWindowRect(ownerHwnd,&rect);
    int             widOwner = rect.right - rect.left,
                    hgtOwner = rect.bottom - rect.top,
                    widDlg = 500,
                    hgtDlg = 200;
    // CreateWindowEx sends WM_CREATE and certain other messages before returning.
    // This is done so that the caller can send messages to the child window immediately
    // after calling this function.
    HWND hwnd =
        CreateWindowEx(
            NULL,
            className.c_str(), 
            title.as_wstring().c_str(),
            // The window has a border and title bar but no min/max or resizing:
            WS_OVERLAPPED | WS_VISIBLE,
            rect.left + (widOwner-widDlg)/2,
            rect.top + (hgtOwner-hgtDlg)/2,
            widDlg,hgtDlg,
            ownerHwnd,
            NULL,               // Use class menu definition (ie none)
            s_fgGuiWin.hinst,   // Contrary to MSDN docs, this is used on all WinOSes to
                                // disambiguate the class name over different modules [Raymond Chen]
            thisPtr);           // Value to be sent as argument with WM_NCCREATE message
    FGASSERTWIN(hwnd != 0);
    return hwnd;
}

FgVect2I
fgScreenPos(HWND hwnd,LPARAM lParam);

FgVect2UI
fgNcSize(HWND hwnd);

#endif

// */
