#pragma once
#include "Cell.h"
#include "BindRow.h"
#include "Direct2DWrite.h"
#include "D2DFileIconDrawer.h"
#include "ShellFile.h"
#include <sigslot/signal.hpp>
#include "KnownFolder.h"
#include "ResourceIDFactory.h"
#include "FilerWnd.h"
#include "FilerApplication.h"
#include "ToolTip.h"

class CFileIconCell:public CCell
{
protected:
	mutable sigslot::connection m_conDelayUpdateAction;
public:
	using CCell::CCell;
	virtual ~CFileIconCell() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile() = 0;

	virtual std::wstring GetString() override
	{
		if (GetShellFile()) {
			return GetShellFile()->GetDispName();
		} else {
			return L"";
		}
	}

	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override
	{
		auto spFile = GetShellFile();
		if (spFile) {
			CRectF rc = rcPaint;
			rc.bottom = rc.top + pDirect->Pixels2DipsY(16);
			rc.right = rc.left + pDirect->Pixels2DipsX(16);

			auto updated = [wp = std::weak_ptr(std::dynamic_pointer_cast<CFileIconCell>(shared_from_this()))]()->void {
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
			pDirect->GetFileIconDrawerPtr()->DrawFileIconBitmap(pDirect, rc.LeftTop(), spFile.get(), updated);
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

	virtual void OnMouseEnter(const MouseEnterEvent& e) override
	{
		::OutputDebugStringA("OnMouseEnter\r\n");
		auto InitialShowDelay = ::GetDoubleClickTime();
		auto BetweenShowDelay = InitialShowDelay / 5;
		auto ShowDuration = InitialShowDelay * 10;

		auto GetCursorSize = []()->CSize {
			return CSize(::GetSystemMetrics(SM_CXCURSOR), ::GetSystemMetrics(SM_CYCURSOR));
		};

	//auto GetCursorIconSize = []()->CSize {
	//	CIcon icon(::GetCursor());
	//	CSize iconSize;
	//	ICONINFO ii;
	//	if (::GetIconInfo(icon, &ii)) {
	//		if (BITMAP bm; ::GetObjectW(ii.hbmMask, sizeof(bm), &bm) == sizeof(bm)) {
	//			iconSize.cx = bm.bmWidth;
	//			iconSize.cy = ii.hbmColor ? bm.bmHeight : bm.bmHeight / 2;
	//		}
	//		if (ii.hbmMask)  ::DeleteObject(ii.hbmMask);
	//		if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
	//	}
	//	return iconSize;
	//};


		auto initialEnter = false;
		auto betweenEnter = true;


	//if (leave) {
	//	if (pSheet->GetWndPtr()->GetToolTipControlPtr()) {
	//		pSheet->GetWndPtr()->GetToolTipControlPtr()->OnClose(CloseEvent(e.WndPtr, NULL, NULL));
	//	}
	//} else if (initialEnter || betweenEnter) {
		auto delay = initialEnter ? InitialShowDelay : BetweenShowDelay;

		if (this->m_pSheet->GetWndPtr()->GetToolTipControlPtr()) {
			this->m_pSheet->GetWndPtr()->GetToolTipControlPtr()->OnClose(CloseEvent(e.WndPtr, NULL, NULL));
		}

		if (!GetString().empty()) {
			e.WndPtr->GetToolTipDeadlineTimer().run([pSheet = this->m_pSheet, content = GetString(), sz = GetCursorSize()]()->void
			{
				auto spTT = std::make_shared<CToolTip>(
					pSheet->GetWndPtr(),
					std::make_shared<ToolTipProperty>());
				CPointF pt = pSheet->GetWndPtr()->GetCursorPosInWnd();
				pt.x += sz.cx / 2;
				spTT->OnCreate(CreateEvt(pSheet->GetWndPtr(), pSheet->GetWndPtr(), CRectF()));
				spTT->Content.set(content);
				spTT->Measure(CSizeF(FLT_MAX, FLT_MAX));
				spTT->Arrange(CRectF(pt, spTT->DesiredSize()));

			}, std::chrono::milliseconds(delay));
		}
	//} 
	}
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override
	{
		e.WndPtr->GetToolTipDeadlineTimer().stop();
		::OutputDebugStringA("OnMouseLeave\r\n");
		if (this->m_pSheet->GetWndPtr()->GetToolTipControlPtr()) {
			this->m_pSheet->GetWndPtr()->GetToolTipControlPtr()->OnClose(CloseEvent(e.WndPtr, NULL, NULL));
		}
	}

};