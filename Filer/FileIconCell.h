#pragma once
#include "Cell.h"
#include "BindRow.h"
#include "Direct2DWrite.h"
#include "FileIconCache.h"
#include "ShellFile.h"

template<typename... TItems>
class CFileIconCell:public CCell, public std::enable_shared_from_this<CFileIconCell<TItems...>>
{
protected:
	mutable boost::signals2::connection m_conDelayUpdateAction;
public:
	using CCell::CCell;
	virtual ~CFileIconCell() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile() = 0;

	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override
	{
		auto spFile = GetShellFile();
		if (spFile) {
			d2dw::CRectF rc = rcPaint;
			rc.bottom = rc.top + pDirect->Pixels2DipsY(16);
			rc.right = rc.left + pDirect->Pixels2DipsX(16);

			std::weak_ptr<CFileIconCell> wp(shared_from_this());
			std::function<void()> updated = [wp]()->void {
				if (auto sp = wp.lock()) {
					auto con = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
						[wp]()->void {
							if (auto sp = wp.lock()) {
								sp->OnPropertyChanged(L"value");
							}
						});
					sp->m_conDelayUpdateAction = con;
					sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
				}
			};
			pDirect->DrawBitmap(pDirect->GetIconCachePtr()->GetFileIconBitmap(spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), updated), rc);
		}
	}

	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* pDirect) override
	{
		return d2dw::CSizeF(pDirect->Pixels2DipsX(16), pDirect->Pixels2DipsY(16));
	}

	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect) override
	{
		return MeasureContentSize(pDirect);
	}

};