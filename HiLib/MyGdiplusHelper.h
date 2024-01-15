#pragma once
#include <objidl.h>
//  Define min max macros required by GDI+ headers.
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#else
#error max macro is already defined
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#else
#error min macro is already defined
#endif

#include <gdiplus.h>

//  Undefine min max macros so they won't collide with <limits> header content.
#undef min
#undef max

#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;

namespace GdiplusHelper
{

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

BOOL SaveBitmapAsBmpFile(std::wstring wstrFilePath, HBITMAP hbm);

BOOL SaveBitmapAsPngFile(std::wstring wstrFilePath, HBITMAP hbm);

BOOL SaveBitmapAsJpegFile(std::wstring wstrFilePath, HBITMAP hbm);

BOOL SaveBitmapAsGifFile(std::wstring wstrFilePath, HBITMAP hbm);

BOOL SaveBitmapAsTiffFile(std::wstring wstrFilePath, HBITMAP hbm);
}