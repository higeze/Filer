#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows
#include <windows.h>
// ATL
#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>

// C header
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#include <d2d1_1.h>
#include <dwrite_1.h>
#include <d2d1helper.h>

// C++ header
#include <string>
#include <stack>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include <fcntl.h>
#include <io.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <sstream>
#include <codecvt>
#include <dispex.h>
#include <limits.h>
#include <functional>
#include <memory>
#include <type_traits>

#include <imm.h>
#include <msctf.h>

#include <msxml6.h>

#include <comutil.h>
#include <mmsystem.h>

//#include "xcom.h"

//#include "FString.h"


#define DEFAULTFONT DEFAULTFONT_JP
#define DEFAULTFONT_HEIGHT DEFAULTFONT_HEIGHT_JP

#define DEFAULTFONT_HEIGHT_WITH_SPACE	18
#define _TAB 9
#define _CR 13
#define _LF 10
#define _PAI 3.14159f
#define CRLF L"\r\n"

#define MAJAR_VERSION 1
#define MINOR_VERSION 0


#define NONAME L"__NONAME"
#define OUT



//
//#ifdef _DEBUG
//#define TRACE FString::debug_trace
//#else
//#define TRACE //
//#endif


#pragma warning(disable:4482) // C4482 enumのms拡張
#pragma warning (disable:4250)

#define TEMP_INVALID_VAL 100000




//#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
//#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#define xassert(x)	_ASSERT(x)
// VARIANT は plValを使用すること
#define VT_F_LPVOID	(VT_BYREF|VT_I4)


using namespace std::placeholders;
typedef _bstr_t bstring;

#define BS(a)	CComBSTR(L##a)


#undef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN7


#include "Direct2DWrite.h"

#define D2RGBA(r,g,b,a) d2dw::CColorF(r/255.0f, g/255.0f, b/255.0f, a/255.0f ) //  薄い(0) <- A <- 濃い(255)
#define D2RGB(r,g,b) d2dw::CColorF(r/255.0f, g/255.0f, b/255.0f, 1.0f )
#define D2DRGB(dw) d2dw::CColorF(((dw&0xFF0000)>>16)/255.0f, ((dw&0x00FF00)>>8)/255.0f, (dw&0xFF)/255.0f, 1.0f )

#define _USE_MATH_DEFINES
#include <math.h>

#include <Wincodec.h>

//#import <msxml6.dll>	// make msxml6.tlh, msxml6.tli 
