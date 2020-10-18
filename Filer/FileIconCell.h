#pragma once
#include "Cell.h"
#include "BindRow.h"
#include "Direct2DWrite.h"
#include "FileIconCache.h"
#include "ShellFile.h"
#include <sigslot/signal.hpp>


template<typename... TItems>
class CFileIconCell:public CCell//, public std::enable_shared_from_this<CFileIconCell<TItems...>>
{
protected:
	mutable sigslot::connection m_conDelayUpdateAction;
public:
	using CCell::CCell;
	virtual ~CFileIconCell() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile() = 0;

	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override
	{
		auto spFile = GetShellFile();
		if (spFile) {
			CRectF rc = rcPaint;
			rc.bottom = rc.top + pDirect->Pixels2DipsY(16);
			rc.right = rc.left + pDirect->Pixels2DipsX(16);

			std::function<void()> updated = [wp = std::weak_ptr(std::dynamic_pointer_cast<CFileIconCell>(shared_from_this()))]()->void {
				if (auto sp = wp.lock()) {
					sp->m_conDelayUpdateAction = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
						[wp]()->void {
							if (auto sp = wp.lock()) {
								sp->OnPropertyChanged(L"value");
							}
						});
					sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
				}
			};
			pDirect->DrawBitmap(pDirect->GetIconCachePtr()->GetFileIconBitmap(spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), updated), rc);
		}
	}

	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override
	{
		return CSizeF(pDirect->Pixels2DipsX(16), pDirect->Pixels2DipsY(16));
	}

	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override
	{
		return MeasureContentSize(pDirect);
	}

};