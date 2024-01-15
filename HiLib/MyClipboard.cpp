#include "MyClipboard.h"
#include "MyGdiplusHelper.h"
#include "Debug.h"
#include <atlcom.h>
#include <string.h>

HANDLE CClipboard::SetDdb(HBITMAP hDDB)
{
	return SetData(CF_BITMAP, hDDB);
}

Status SaveBitmapToStream(LPCWSTR mimetype, CLSID clsid, Bitmap& bitmap, CComPtr<IStream>& pStream)
{
	if(_tcsicmp(mimetype, L"image/jpeg")==0){
		EncoderParameters encs[1];
		ULONG quality = 80;
		encs->Count = 1;

		encs->Parameter[0].Guid = EncoderQuality;
		encs->Parameter[0].NumberOfValues = 1;
		encs->Parameter[0].Type = EncoderParameterValueTypeLong;
		encs->Parameter[0].Value = &quality;
		return bitmap.Save(pStream, &clsid, encs);
	}else if(_tcsicmp(mimetype, L"image/tiff")==0){
		EncoderParameters encs[2];
		ULONG depth = 24;
		ULONG compression = EncoderValueCompressionLZW;
		encs->Count = 2;

		encs->Parameter[0].Guid = EncoderColorDepth;
		encs->Parameter[0].NumberOfValues = 1;
		encs->Parameter[0].Type = EncoderParameterValueTypeLong;
		encs->Parameter[0].Value = &depth;

		encs->Parameter[1].Guid = EncoderCompression;
		encs->Parameter[1].NumberOfValues = 1;
		encs->Parameter[1].Type = EncoderParameterValueTypeLong;
		encs->Parameter[1].Value = &compression;

		return bitmap.Save(pStream, &clsid, encs);
	}else{
		return bitmap.Save(pStream, &clsid, NULL);
	}
}

HANDLE CClipboard::SetImageFromDdb(LPCWSTR mimetype, LPCWSTR format, HBITMAP hDDB)
{
	//Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CComPtr<IStream> pStream = NULL;
	FAILED_THROW(::CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pStream));

	Bitmap bitmap(hDDB, (HPALETTE)GetStockObject(DEFAULT_PALETTE));

	CLSID clsid;	
	FALSE_THROW(GdiplusHelper::GetEncoderClsid(mimetype, &clsid) >= 0);

	FALSE_THROW(SaveBitmapToStream(mimetype, clsid, bitmap, pStream) == Status::Ok);

	HGLOBAL hGlobal = NULL;
	FAILED_THROW(::GetHGlobalFromStream(pStream, &hGlobal));

	//Copy to Clipboard
	if(_tcsicmp(mimetype, L"image/tiff")==0){
		return SetData(CF_TIFF, hGlobal);
	}else{
		return SetData(::RegisterClipboardFormat(format), hGlobal);
	}
}

HANDLE CClipboard::SetJpegFromDdb(HBITMAP hDDB)
{
	return SetImageFromDdb(L"image/jpeg", L"JFIF", hDDB);
}

HANDLE CClipboard::SetPngFromDdb(HBITMAP hDDB)
{
	return SetImageFromDdb(L"image/png", L"PNG", hDDB);
}

HANDLE CClipboard::SetGifFromDdb(HBITMAP hDDB)
{
	return SetImageFromDdb(L"image/gif", L"GIF", hDDB);
}

HANDLE CClipboard::SetTiffFromDdb(HBITMAP hDDB)
{
	return SetImageFromDdb(L"image/tiff", L"TIFF", hDDB);
}