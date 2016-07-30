//
// application.cpp
//
// (C) Copyright 2000 Jan van den Baard.
//     All Rights Reserved.
//

#include "application.h"
#include "windows/window.h"
#include "windows/mdiwindow.h"
#include "gdi/gdiobject.h"
#include "gdi/dc.h"
#include "menus/menu.h"
#include "menus/bitmapmenu.h"
#include "exceptions/memoryexception.h"
#include "tools/theming.h"
#include "tools/module.h"
#include "tools/xpcolors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Theming API and office XP color support
ClsUxTheme ThemingAPI;
ClsXPColors XPColors;

// Load libraries.
ClsModule mod( _T( "msimg32.dll" ));

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW   0x00020000
#endif

// Horizontal splitter cursor.
static BYTE hSplit_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x00, 0x00, 
	0x00, 0x02, 0x80, 0x00, 
	0x00, 0x04, 0x40, 0x00, 
	0x00, 0x08, 0x20, 0x00, 
	0x00, 0x06, 0xc0, 0x00, 
	0x01, 0xfe, 0xff, 0x00, 
	0x01, 0x00, 0x01, 0x00, 
	0x01, 0xff, 0xff, 0x00, 
	0x01, 0xff, 0xff, 0x00, 
	0x01, 0x00, 0x01, 0x00, 
	0x01, 0xfe, 0xff, 0x00, 
	0x00, 0x06, 0xc0, 0x00, 
	0x00, 0x08, 0x20, 0x00, 
	0x00, 0x04, 0x40, 0x00, 
	0x00, 0x02, 0x80, 0x00, 
	0x00, 0x01, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

static BYTE hSplit_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xfe, 0xff, 0xff, 
	0xff, 0xfc, 0x7f, 0xff, 
	0xff, 0xf8, 0x3f, 0xff, 
	0xff, 0xf0, 0x1f, 0xff, 
	0xff, 0xf8, 0x3f, 0xff, 
	0xfe, 0x00, 0x00, 0xff, 
	0xfe, 0x00, 0x00, 0xff, 
	0xfe, 0x00, 0x00, 0xff, 
	0xfe, 0x00, 0x00, 0xff, 
	0xfe, 0x00, 0x00, 0xff, 
	0xfe, 0x00, 0x00, 0xff, 
	0xff, 0xf8, 0x3f, 0xff, 
	0xff, 0xf0, 0x1f, 0xff, 
	0xff, 0xf8, 0x3f, 0xff, 
	0xff, 0xfc, 0x7f, 0xff, 
	0xff, 0xfe, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Vertical splitter cursor.
static BYTE vSplit_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x07, 0xe0, 0x00, 
	0x00, 0x05, 0xa0, 0x00, 
	0x00, 0x05, 0xa0, 0x00, 
	0x00, 0x05, 0xa0, 0x00, 
	0x00, 0x25, 0xa4, 0x00, 
	0x00, 0x55, 0xaa, 0x00, 
	0x00, 0x9d, 0xb9, 0x00, 
	0x01, 0x01, 0x80, 0x80, 
	0x00, 0x9d, 0xb9, 0x00, 
	0x00, 0x55, 0xaa, 0x00, 
	0x00, 0x25, 0xa4, 0x00, 
	0x00, 0x05, 0xa0, 0x00, 
	0x00, 0x05, 0xa0, 0x00, 
	0x00, 0x05, 0xa0, 0x00, 
	0x00, 0x07, 0xe0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

static BYTE vSplit_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 
	0xff, 0xd8, 0x1b, 0xff, 
	0xff, 0x88, 0x11, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xfe, 0x00, 0x00, 0x7f, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x88, 0x11, 0xff, 
	0xff, 0xd8, 0x1b, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Hand cursor.
static BYTE Hand_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x00, 
	0x03, 0x60, 0x00, 0x00, 
	0x03, 0x6c, 0x00, 0x00, 
	0x03, 0x6d, 0x00, 0x00, 
	0x03, 0x6d, 0x80, 0x00, 
	0x33, 0xfd, 0x80, 0x00, 
	0x3b, 0xff, 0x80, 0x00, 
	0x1b, 0xff, 0x80, 0x00, 
	0x0b, 0xff, 0x80, 0x00, 
	0x0f, 0xff, 0x80, 0x00, 
	0x07, 0xff, 0x80, 0x00, 
	0x07, 0xff, 0x00, 0x00, 
	0x03, 0xff, 0x00, 0x00, 
	0x03, 0xff, 0x00, 0x00, 
	0x01, 0xfe, 0x00, 0x00, 
	0x01, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

static BYTE Hand_and[] =
{
	0xfc, 0xff, 0xff, 0xff, 
	0xf8, 0x7f, 0xff, 0xff, 
	0xf8, 0x7f, 0xff, 0xff, 
	0xf8, 0x7f, 0xff, 0xff, 
	0xf8, 0x7f, 0xff, 0xff, 
	0xf8, 0x1f, 0xff, 0xff, 
	0xf8, 0x03, 0xff, 0xff, 
	0xf8, 0x00, 0xff, 0xff, 
	0xf8, 0x00, 0x7f, 0xff, 
	0x88, 0x00, 0x3f, 0xff, 
	0x80, 0x00, 0x3f, 0xff, 
	0x80, 0x00, 0x3f, 0xff, 
	0xc0, 0x00, 0x3f, 0xff, 
	0xe0, 0x00, 0x3f, 0xff, 
	0xe0, 0x00, 0x3f, 0xff, 
	0xf0, 0x00, 0x3f, 0xff, 
	0xf0, 0x00, 0x7f, 0xff, 
	0xf8, 0x00, 0x7f, 0xff, 
	0xf8, 0x00, 0x7f, 0xff, 
	0xfc, 0x00, 0xff, 0xff, 
	0xfc, 0x00, 0xff, 0xff, 
	0xfc, 0x00, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Dropdown arrow icon.
BYTE DD_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE DD_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0x00, 0x03, 0xff, 
	0xff, 0x00, 0x03, 0xff, 
	0xff, 0xc0, 0x0f, 0xff, 
	0xff, 0xc0, 0x0f, 0xff, 
	0xff, 0xf0, 0x3f, 0xff, 
	0xff, 0xf0, 0x3f, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Dropdown arrow icon (small)
BYTE DDS_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE DDS_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xc0, 0x0f, 0xff, 
	0xff, 0xc0, 0x0f, 0xff, 
	0xff, 0xf0, 0x3f, 0xff, 
	0xff, 0xf0, 0x3f, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Checkmark icon.
BYTE CHK_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE CHK_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xfc, 0xff, 
	0xff, 0xff, 0xfc, 0xff, 
	0xff, 0xff, 0xf0, 0xff, 
	0xff, 0xff, 0xf0, 0xff, 
	0xff, 0xcf, 0xc3, 0xff, 
	0xff, 0xcf, 0xc3, 0xff, 
	0xff, 0xc3, 0x0f, 0xff, 
	0xff, 0xc3, 0x0f, 0xff, 
	0xff, 0xf0, 0x3f, 0xff, 
	0xff, 0xf0, 0x3f, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Close-button icon.
BYTE Close_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE Close_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0x0f, 0xf0, 0xff, 
	0xff, 0x0f, 0xf0, 0xff, 
	0xff, 0xc3, 0xc3, 0xff, 
	0xff, 0xc3, 0xc3, 0xff, 
	0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xc3, 0xc3, 0xff, 
	0xff, 0xc3, 0xc3, 0xff, 
	0xff, 0x0f, 0xf0, 0xff, 
	0xff, 0x0f, 0xf0, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Left arrow icon.
BYTE Left_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE Left_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0x3f, 0xff, 
	0xff, 0xff, 0x3f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xf0, 0x00, 0xff, 
	0xff, 0xf0, 0x00, 0xff, 
	0xff, 0xc0, 0x00, 0xff, 
	0xff, 0xc0, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0xc0, 0x00, 0xff, 
	0xff, 0xc0, 0x00, 0xff, 
	0xff, 0xf0, 0x00, 0xff, 
	0xff, 0xf0, 0x00, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xff, 0x3f, 0xff, 
	0xff, 0xff, 0x3f, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Up arrow icon.
BYTE Up_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE Up_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xfc, 0x00, 0x00, 0x3f, 
	0xfc, 0x00, 0x00, 0x3f, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Right arrow icon.
BYTE Right_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE Right_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0x00, 0x0f, 0xff, 
	0xff, 0x00, 0x0f, 0xff, 
	0xff, 0x00, 0x03, 0xff, 
	0xff, 0x00, 0x03, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x03, 0xff, 
	0xff, 0x00, 0x03, 0xff, 
	0xff, 0x00, 0x0f, 0xff, 
	0xff, 0x00, 0x0f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Down arrow icon.
BYTE Down_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE Down_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xfc, 0x00, 0x00, 0x3f, 
	0xfc, 0x00, 0x00, 0x3f, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0x00, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 
	0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Pin icon.
BYTE Pin_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE Pin_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xc0, 0x0f, 0xff, 
	0xff, 0xc0, 0x0f, 0xff, 
	0xff, 0xcf, 0x0f, 0xff, 
	0xff, 0xcf, 0x0f, 0xff, 
	0xff, 0xcf, 0x0f, 0xff, 
	0xff, 0xcf, 0x0f, 0xff, 
	0xff, 0xcf, 0x0f, 0xff, 
	0xff, 0xcf, 0x0f, 0xff, 
	0xff, 0x00, 0x03, 0xff, 
	0xff, 0x00, 0x03, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Pinned icon.
BYTE Pinned_xor[] =
{
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
};

BYTE Pinned_and[] =
{
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xf3, 0xff, 0xff, 
	0xff, 0xf3, 0xff, 0xff, 
	0xff, 0xf0, 0x03, 0xff, 
	0xff, 0xf0, 0x03, 0xff, 
	0xff, 0xf3, 0xf3, 0xff, 
	0xff, 0xf3, 0xf3, 0xff, 
	0xfc, 0x03, 0xf3, 0xff, 
	0xfc, 0x03, 0xf3, 0xff, 
	0xff, 0xf0, 0x03, 0xff, 
	0xff, 0xf0, 0x03, 0xff, 
	0xff, 0xf0, 0x03, 0xff, 
	0xff, 0xf0, 0x03, 0xff, 
	0xff, 0xf3, 0xff, 0xff, 
	0xff, 0xf3, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
};

// Defined in "window.cpp", "gdiobject.cpp", "dc.cpp",
// "menu.cpp".
extern ClsLinkedList<ClsWindow>		global_window_list;
extern ClsLinkedList<ClsGdiObject>	global_gdi_objects;
extern ClsLinkedList<ClsDC>		global_dc_list;
extern ClsLinkedList<ClsMenu>		global_menu_list;

// Defined in "gdiobject.cpp" and "dc.cpp"
#ifdef _DEBUG
extern int nGDIObjects;
extern int nDCObjects;
#endif

// Loaded common controls.
static DWORD dwCommonControls = 0;

// One application object. Use the ClsGetApp() function
// to get/create it.
ClsApp *global_application_object = NULL;

// atexit() registered routine which makes sure the
// global application object is freed and the temporary
// objects are freed when the app terminates.
void atexit_ClsApp()
{
	_ASSERT_TEXT( global_gdi_objects.GetSize() == 0, "Global gdi list not empty." ); // Must be empty.
	_ASSERT_TEXT( global_window_list.GetSize() == 0, "Global window list not empty." ); // Must be empty.
	_ASSERT_TEXT( global_dc_list.GetSize() == 0, "Global device context list not empty." ); // Must be empty.
	_ASSERT_TEXT( global_menu_list.GetSize() == 0, "Global menu list not empty." ); // Must be empty.
	_ASSERT_TEXT( nGDIObjects == 0, "Did not destroy all GDI objects." ); // Should be zero.
	_ASSERT_TEXT( nDCObjects == 0, "Did not destroy all device contexts." ); // Should be zero.

	// Remove all temporary (bitmap)menu objects.
	ClsBitmapMenu::DeleteTempObjects();
	ClsMenu::DeleteTempObjects();

	// Remove all temporary dc objects.
	ClsDC::DeleteTempObjects();

	// Remove all temporary window objects.
	ClsWindow::DeleteTempObjects();

	// Make sure the temporary gdi objects are deleted.
	ClsGdiObject::DeleteTempObjects();

	// Does the global application object
	// exist?
	if ( global_application_object )
		// Free it.
		delete global_application_object;

	// Should not assert here.
	_TEST_HEAP;
}

// A new handler which will throw a ClsMemoryException.
int NewHandler( size_t size )
{
	// Throw a memory exception.
	throw ClsMemoryException();
}

// Misc. functions...
HINSTANCE ClsGetInstanceHandle()	{ return ClsGetApp()->GetInstanceHandle(); }
HINSTANCE ClsGetResourceHandle()	{ return ClsGetApp()->GetResourceHandle(); }
HCURSOR ClsGetCursor( int nCursor )	{ return ClsGetApp()->GetCursor( nCursor ); }

// Get a pointer to the global application object. If it
// does not exist yet we create it.
ClsApp *ClsGetApp()
{
	// Does it already exist?
	if ( global_application_object == NULL )
	{
		// Setup the new handler.
		_set_new_handler( NewHandler );

		// No. Create it.
		global_application_object = new ClsApp;

		// Register the atexit() function so that
		// it does get deleted properly at program
		// termination.
		atexit( atexit_ClsApp );
	}

	// Must be valid here.
	_ASSERT_VALID( global_application_object );

	// Return the global application object.
	return global_application_object;
}

#ifdef _DEBUG
// Replacements for the new and delete operators. These will report the
// file name and line number where blocks of unfreed memory where
// allocated.
#undef new
void *operator new( size_t nSize, const char *lpszFileName, int nLine )
{
	// Allocate the data.
	void *pData = _malloc_dbg( nSize, _CLIENT_BLOCK, lpszFileName, nLine );

	// Failed?
	if ( pData == NULL )
	{
		// Call the new handler which, in the case
		// of this library, will throw an exception.
		_PNH nh = _set_new_handler( 0 );
		( *nh )( nSize );
		_set_new_handler( nh );
	}
	// Return pointer or NULL.
	return pData;
}

void operator delete( void *pData, const char *lpszFileName, int nLine )
{
	// Free the data.
	_free_dbg( pData, _CLIENT_BLOCK );
}

void ClsTrace( LPCTSTR pszFormatStr, ... )
{
	TCHAR		szBuffer[ 512 ]; // 512 chars is max.

	// Format the output.
	va_list		args;
	va_start( args, pszFormatStr );
	::wvsprintf( szBuffer, pszFormatStr, args );
	va_end( args );

	// Allocate a buffer to fit.
	::OutputDebugString( szBuffer );
}
#endif

// Constructor.
ClsApp::ClsApp()
{
	// Clear the data fields.
	m_hInstance	    = NULL;
	m_hResources	    = NULL;
	m_hSystemImages	    = NULL;
	m_hAtom		    = NULL;
	m_hIcon		    = NULL;
	m_bPrivClass	    = FALSE;
	m_hPattern	    = NULL;
	m_hPatternBrush     = NULL;
	m_hImages	    = NULL;
	m_hRichedit	    = NULL;

	for ( int i = 0; i < CII_NUMICONS; i++ )
		m_hIcons[ i ] = NULL;
	for ( int i = 0; i < STC_NUMCURSORS; i++ )
		m_hCursors[ i ] = NULL;
}

// Destructor.
ClsApp::~ClsApp()
{
	ATOM	hResult = 0;

	// Delete cursors.
	int i;
	for ( i = 0; i < STC_NUMCURSORS; i++ )
		if ( m_hCursors[ i ] ) ::DestroyCursor( m_hCursors[ i ] );

	// Delete the create atom.
	if ( m_hAtom ) 
		hResult = ::GlobalDeleteAtom( m_hAtom );

	// Delete the accelerator tables.
	for ( i = 0; i < m_Accelerators.GetSize(); i++ )
		::DestroyAcceleratorTable( m_Accelerators[ i ].m_hAccel );

	// Destroy the icons.
	if ( m_hIcon      ) ::DestroyIcon( m_hIcon );
	if ( m_hIconSmall ) ::DestroyIcon( m_hIconSmall );
	for ( i = 0; i < CII_NUMICONS; i++ )
		if ( m_hIcons[ i ] ) ::DestroyIcon( m_hIcons[ i ] );

	// Unregister the private classes.
	if ( m_bPrivClass )
	{
		UnregisterClass( _T( "ClsWindowClass" ),   GetInstanceHandle());
		UnregisterClass( _T( "ClsMDIChildClass" ), GetInstanceHandle());
		UnregisterClass( _T( "ClsDSWindowClass" ), GetInstanceHandle());
	}

	// Unregister the classes.
	for ( i = 0; i < m_aClasses.GetSize(); i++ )
		UnregisterClass( m_aClasses[ i ], GetInstanceHandle());

	// Delete pattern.
	if ( m_hPatternBrush )
		::DeleteObject( m_hPatternBrush );
	if ( m_hPattern )
		::DeleteObject( m_hPattern );

	// Destroy imagelist.
	if ( m_hImages )
		ImageList_Destroy( m_hImages );

	// Closeup the Richedit DLL.
	if ( m_hRichedit )
		::FreeLibrary( m_hRichedit );

	// Remove the message hook.
	ClsMenu::RemoveMessageHook();

	// Must be zero!
	_ASSERT( hResult == 0 );
}

// Setup the application object.
BOOL ClsApp::Setup( HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow, LPCTSTR pszIconName /* = NULL */, LPCTSTR pszAppTitle /* = NULL */, WNDCLASSEX *pWndClass /* = NULL */, int nClassCount /* = 0 */, DWORD dwControls /* = 0 */ )
{
	_ASSERT_VALID( hInstance ); // Must be valid.

	// Save command line and show command.
	m_lpCmdLine = lpCmdLine;
	m_nCmdShow  = nCmdShow;

	// Get the OS version information.
	m_osVersion.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	if ( GetVersionEx( &m_osVersion ))
	{
		// Setup the instance handle.
		m_hInstance = hInstance; 

		// Install the menu hook.
		ClsMenu::InstallMessageHook();

		// Load an icon?
		if ( pszIconName )
		{
			// Load the icons.
			if (( m_hIcon = ( HICON )::LoadImage( GetResourceHandle(), pszIconName, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR )) == FALSE )
				return FALSE;
			if (( m_hIconSmall = ( HICON )::LoadImage( GetResourceHandle(), pszIconName, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR )) == FALSE )
				return FALSE;
		}
		else
			// Use the default.
			m_hIcon = m_hIconSmall = ::LoadIcon( NULL, IDI_APPLICATION );

		// Get common controls version.
		ComCtlVersion();

		// Any title specified?
		if ( pszAppTitle )
		{
			// When the high word of the title pointer
			// is 0 it must be loaded from the resources.
			if ( HIWORD( pszAppTitle ) == 0 )
			{
				// Load the title from the resources.
				if ( m_sAppTitle.LoadString(( UINT )LOWORD(pszAppTitle)) == FALSE )
					return FALSE;
			}
			else
				// Setup the title string.
				m_sAppTitle = pszAppTitle;
		}

		// Get system image list.
		SHFILEINFO fi;
		m_hSystemImages = ( HIMAGELIST )::SHGetFileInfo( _T( "C:\\" ), 0, &fi, sizeof( fi ), SHGFI_SYSICONINDEX | SHGFI_SMALLICON );
		if ( m_hSystemImages == NULL )
			return FALSE;

		// Create cursors.
		m_hCursors[ STC_HAND   ] = ::CreateCursor( ClsGetInstanceHandle(),  5,  0, 32, 32, Hand_and,   Hand_xor );
		m_hCursors[ STC_HSPLIT ] = ::CreateCursor( ClsGetInstanceHandle(), 15, 15, 32, 32, hSplit_and, hSplit_xor );
		m_hCursors[ STC_VSPLIT ] = ::CreateCursor( ClsGetInstanceHandle(), 15, 15, 32, 32, vSplit_and, vSplit_xor );

		// Create icons.
		m_hIcons[ CII_DROPDOWN	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, DD_and, DD_xor );
		m_hIcons[ CII_DROPDOWN_SMALL ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, DDS_and, DDS_xor );
		m_hIcons[ CII_CHECKMARK	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, CHK_and, CHK_xor );
		m_hIcons[ CII_CLOSE	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, Close_and, Close_xor );
		m_hIcons[ CII_UP	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, Up_and, Up_xor );
		m_hIcons[ CII_DOWN	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, Down_and, Down_xor );
		m_hIcons[ CII_LEFT	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, Left_and, Left_xor );
		m_hIcons[ CII_RIGHT	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, Right_and, Right_xor );
		m_hIcons[ CII_PIN	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, Pin_and, Pin_xor );
		m_hIcons[ CII_PINNED	     ] = ::CreateIcon( ClsGetInstanceHandle(), 32, 32, 1, 1, Pinned_and, Pinned_xor );

		// Check cursors.
		BOOL bGfxHandles = TRUE;
		for ( int i = 0; i < STC_NUMCURSORS; i++ )
		{
			if ( ! m_hCursors[ i ] )
			{
				bGfxHandles = FALSE;
				break;
			}
		}
		// And icons.
		if ( bGfxHandles )
		{
			for ( int i = 0; i < CII_NUMICONS; i++ )
			{
				if ( ! m_hIcons[ i ] )
				{
					bGfxHandles = FALSE;
					break;
				}
			}
		}

		// OK?
		if ( bGfxHandles == TRUE )
		{
			// Create the image list.
			if (( m_hImages = ImageList_Create( 16, 16, ILC_MASK, CII_NUMICONS, 0 )) == NULL )
				return FALSE;

			// Add the icons to it.
			for ( int i = 0; i < CII_NUMICONS; i++ )
			{
				if ( ImageList_AddIcon( m_hImages, m_hIcons[ i ] ) < 0 )
					return FALSE;
			}

			// First we register our own, private
			// window class.
			WNDCLASSEX	wClass;
			wClass.cbSize		= sizeof( wClass );
			wClass.cbClsExtra	= 0;
			wClass.cbWndExtra	= 0;
			wClass.lpszMenuName	= NULL;
			wClass.hbrBackground	= ::GetSysColorBrush( COLOR_WINDOW );
			wClass.hCursor		= ::LoadCursor( NULL, IDC_ARROW );
			wClass.hIcon		= wClass.hIconSm = m_hIcon;
			wClass.hInstance	= hInstance;
			wClass.lpfnWndProc	= ClsWindow::StaticWindowProc;
			wClass.lpszClassName	= _T( "ClsWindowClass" );
			wClass.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

			// Register the class.
			if ( ! RegisterClassEx( &wClass ))
				return FALSE;
			
			// Also register the MDI child class.
			wClass.lpszClassName	= _T( "ClsMDIChildClass" );
			wClass.lpfnWndProc	= ClsMDIChildWindow::StaticMDIChildProc;

			// Register it.
			if ( ! RegisterClassEx( &wClass ))
			{
				UnregisterClass( _T( "ClsWindowClass" ), hInstance );
				return FALSE;
			}

			// Register the class with a drop-shadow window.
			wClass.lpszClassName = _T( "ClsDSWindowClass" );
			wClass.lpfnWndProc   = ClsWindow::StaticWindowProc;
			wClass.style	    |= CS_DROPSHADOW;

			// Register it...
			if ( ! RegisterClassEx( &wClass ))
			{
				// No drop-shadow support...
				wClass.style &= ~CS_DROPSHADOW;
				if ( ! RegisterClassEx( &wClass ))
				{
					// Fail...
					UnregisterClass( _T( "ClsWindowClass" ), hInstance );
					UnregisterClass( _T( "ClsMDIChildClass" ), hInstance );
					return FALSE;
				}
			}

			// Our own classes are registered.
			m_bPrivClass = TRUE;

			// Are there any classes we must register?
			if ( pWndClass )
			{
				// Iterate classes to register.
				for ( int i = 0; i < nClassCount; i++, pWndClass++ )
				{
					// Make sure it uses the correct instance handle.
					pWndClass->hInstance = hInstance;

					// If we do not have a valid window class
					// procedure we use the default ClsWindow
					// procedure.
					if ( pWndClass->lpfnWndProc == NULL )
						pWndClass->lpfnWndProc = ( WNDPROC )ClsWindow::StaticWindowProc;
					
					// If an icon was not provided we set it
					// to the application icon.
					if ( pWndClass->hIcon   == NULL ) pWndClass->hIcon   = m_hIcon;
					if ( pWndClass->hIconSm == NULL ) pWndClass->hIconSm = m_hIcon;

					// Register the class.
					if ( RegisterClassEx( pWndClass ) == NULL )
						return FALSE;

					// Save the name so we can unregister
					// the class.
					ClsString sClassName( pWndClass->lpszClassName );
					m_aClasses.Add( sClassName );
				}
			}
		}
		else
			return FALSE;

		// Setup the controls.
		if ( dwControls ) AddCommonControls( dwControls );

		// Create pattern.
		static WORD aPattern[ 8 ] =
		{
			170, 85,
			170, 85,
			170, 85,
			170, 85
		};

		// Create the bitmap.
		if (( m_hPattern = CreateBitmap( 8, 8, 1, 1, &aPattern )) == NULL )
			return FALSE;

		// Create the brush.
		if (( m_hPatternBrush = ::CreatePatternBrush( m_hPattern )) == NULL )
			return FALSE;
		return TRUE;
	}
	return FALSE;
}

// Test to see if we are already running.
BOOL ClsApp::AreWeRunning( LPCTSTR pszAtomName )
{
	// See if the ATOM already exists.
	if ( GlobalFindAtom( pszAtomName ) == NULL )
	{
		// Create the ATOM.
		m_hAtom = GlobalAddAtom( pszAtomName );
		if ( m_hAtom )
			// Return FALSE which means that the application
			// was not run yet.
			return FALSE;
	}
	// Return TRUE which means that the ATOM already
	// exists (I.E. the app is already running.
	return TRUE;
}

// Return the resources handle.
HINSTANCE ClsApp::GetResourceHandle()
{
	_ASSERT_VALID( m_hInstance ); // Must be valid...

	// Is the resource handle valid?
	if ( m_hResources == NULL )
		// Set it to the instance handle.
		m_hResources = m_hInstance;
	
	// Return the handle.
	return m_hResources;
}

// Load an accelerator table.
BOOL ClsApp::LoadAcceleratorTable( HWND hWnd, LPCTSTR pszAccelName )
{
	// Fill in structure.
	AccelWnd aw;
	aw.m_hWnd   = hWnd;
	aw.m_hAccel = ::LoadAccelerators( ClsGetResourceHandle(), pszAccelName );

	// Accelerators loaded?
	if ( aw.m_hAccel )
	{
		// Add to the accelerator array.
		if ( m_Accelerators.Add( aw ) >= 0 )
			return TRUE;

		// Detroy table.
		DestroyAcceleratorTable( aw.m_hAccel );
	}
	return FALSE;
}

// Translate an accelerator.
BOOL ClsApp::TranslateAccelerator( LPMSG pMsg )
{
	_ASSERT_VALID( pMsg ); // Must be valid.

	// Iterate accelerator table.
	for ( int i = 0; i < m_Accelerators.GetSize(); i++ )
	{
		// Is this the one?
		if ( ::TranslateAccelerator( m_Accelerators[ i ].m_hWnd, m_Accelerators[ i ].m_hAccel, pMsg ))
			return TRUE;
	}
	return FALSE;
}

// Add a common control type.
void ClsApp::AddCommonControls( DWORD dwCtrlType )
{
	// Is the control type already loaded?
	if ( dwCtrlType == 0 || (( dwCommonControls & dwCtrlType ) == dwCtrlType ))
		return;

	// Get the library.
	HMODULE hCommCtrl = GetModuleHandle( _T( "comctl32.dll" ));
	if ( hCommCtrl )
	{
		// See if the InitCommonControlsEx() API is available.
		if ( GetProcAddress( hCommCtrl, "InitCommonControlsEx" ))
		{
			// Yes. We can use this API.
			INITCOMMONCONTROLSEX	ccex;
			ccex.dwSize = sizeof( ccex );
			ccex.dwICC  = dwCtrlType;

			// Initialize...
			InitCommonControlsEx( &ccex );

			// Add bits.
			dwCommonControls |= dwCtrlType;
		}
		else
		{
			// Initialize 'm all.
			InitCommonControls();
			dwCommonControls = ( DWORD )-1;
		}
	}
}

// Determine common controls version.
void ClsApp::ComCtlVersion()
{
	// Setup defaults.
	m_nVer = m_nRev = 0;

	// Loadup the library.
	HMODULE hMod = ::GetModuleHandle( _T( "comctl32.dll" ));
	if ( hMod )
	{
		// Get the version function.
		DLLGETVERSIONPROC	pDllGetVersion;
		pDllGetVersion = ( DLLGETVERSIONPROC )GetProcAddress( hMod, "DllGetVersion" );

		// Is it there?
		if ( pDllGetVersion )
		{
			// Obtain version information.
			DLLVERSIONINFO		dvi;
			ZeroMemory( &dvi, sizeof( dvi ));
			dvi.cbSize = sizeof ( dvi );
			if ( SUCCEEDED(( *pDllGetVersion )( &dvi )))
			{
				// Save the information.
				m_nVer = dvi.dwMajorVersion;
				m_nRev = dvi.dwMinorVersion;
			}
		}
	}
}

// Menu fading enabled?
#ifndef SPI_GETMENUFADE
#define SPI_GETMENUFADE 0x1012
#endif

BOOL ClsApp::IsMenuFadeEnabled()
{
	BOOL bEnabled = FALSE;
	if ( ::SystemParametersInfo( SPI_GETMENUFADE, 0, &bEnabled, 0 ))
		return bEnabled;
	return FALSE;
}

// Is shadowing enabled by the system?
#ifndef SPI_GETDROPSHADOW
#define SPI_GETDROPSHADOW 0x1024
#endif

BOOL ClsApp::IsShadowEnabled()
{
	BOOL bEnabled = FALSE;
	if ( ::SystemParametersInfo( SPI_GETDROPSHADOW, 0, &bEnabled, 0 ))
		return bEnabled;
	return FALSE;
}

// Initialize the Richedit20 DLL.
BOOL ClsApp::InitRichedit()
{
	// Already loaded?
	if ( m_hRichedit == NULL )
		m_hRichedit = ::LoadLibrary( _T( "riched20.dll"));
	return ( BOOL )( m_hRichedit ? TRUE : FALSE );
}