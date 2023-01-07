#pragma once
#include "Cell.h"
#include "BindRow.h"
#include "Direct2DWrite.h"
#include "ShellFile.h"
#include <sigslot/signal.hpp>
#include <thumbcache.h>
#include <ShlGuid.h>
#include "Debug.h"

#include "D2DThumbnailDrawer.h"


template<typename... TItems>
class CFileThumbnailCell:public CCell
{
protected:
	mutable sigslot::connection m_conDelayUpdateAction;
	int m_size = 64;
public:
	using CCell::CCell;
	virtual ~CFileThumbnailCell() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile()
	{
		if (auto pBindRow = dynamic_cast<CBindRow<TItems...>*>(m_pRow)) {
			return std::get<std::shared_ptr<CShellFile>>(pBindRow->GetTupleItems());
		} else {
			return nullptr;
		}
	}

	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override
	{
		auto spFile = GetShellFile();
		if (spFile) {
			//CComPtr<IShellItem2> pShellItem;
			//FAILED_RETURN(::SHCreateItemFromIDList(spFile->GetAbsoluteIdl().ptr(), IID_PPV_ARGS(&pShellItem)));

			pDirect->GetFileThumbnailDrawerPtr()->DrawThumbnailBitmap(pDirect, spFile->GetPath(), rcPaint, m_size);
			//CRectF rc = rcPaint;
			//rc.bottom = rc.top + pDirect->Pixels2DipsY(m_size);
			//rc.right = rc.left + pDirect->Pixels2DipsX(m_size);

			//CComPtr<IShellItem2> pShellItem;
			//FAILED_RETURN(::SHCreateItemFromIDList(spFile->GetAbsoluteIdl().ptr(), IID_PPV_ARGS(&pShellItem)));

			//CComPtr<IThumbnailCache> pThumbCache;
			//pThumbCache.CoCreateInstance(CLSID_LocalThumbnailCache, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER);
			//CComPtr<ISharedBitmap> pSharedBitmap;
			//FAILED_RETURN(pThumbCache->GetThumbnail(pShellItem, m_size, WTS_EXTRACT | WTS_SCALETOREQUESTEDSIZE, &pSharedBitmap, NULL, NULL));
			//HBITMAP hBitmap;
			//FAILED_RETURN(pSharedBitmap->GetSharedBitmap(&hBitmap));//This HBitmap is in shared memory, no need to DeleteObject
			
			//CComPtr<IThumbnailProvider> pThumbProvider;
			//FAILED_RETURN( pShellItem->BindToHandler(NULL, BHID_ThumbnailHandler, IID_PPV_ARGS(&pThumbProvider)));
			//WTS_ALPHATYPE wtsAlpha;
			//HBITMAP hBitmap;
			//FAILED_RETURN(pThumbProvider->GetThumbnail(m_size, &hBitmap, &wtsAlpha));

			//std::unique_ptr<std::remove_pointer_t<HBITMAP>, delete_object>  pBmp(hBitmap);
			//CComPtr<IWICBitmap> pWICBitmap;
			//FAILED_RETURN(pDirect->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));
			//CComPtr<ID2D1Bitmap> pBitmap;
			//FAILED_RETURN(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));

			//CPointF leftTop = rcPaint.LeftTop();
			//CSizeF size = pBitmap->GetSize();
			//CRectF rc(leftTop.x, leftTop.y, leftTop.x + size.width, leftTop.y + size.height);
			//pDirect->DrawBitmap(pBitmap, rc);
		}
	}

	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override
	{
		return CSizeF(pDirect->Pixels2DipsX(m_size), pDirect->Pixels2DipsY(m_size));
	}

	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override
	{
		return MeasureContentSize(pDirect);
	}

};