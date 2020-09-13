#pragma once
#include "Sheet.h"
#include "GridView.h"
#include "Column.h"

//Pre-Declaration
class CSheet;
struct RowTag;
struct ColTag;
struct MouseEvent;
struct SetCursorEvent;

class ITracker
{
public:
	virtual void OnDividerDblClk(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnBeginTrack(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnTrack(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnEndTrack(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnLeaveTrack(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnSetCursor(CSheet* pSheet, const SetCursorEvent& e) = 0;
	virtual bool IsTarget(CSheet* pSheet, const MouseEvent& e) = 0;
};

template<typename TRC>
class CTracker:public ITracker
{
private:
	int m_trackLeftVisib;
public:
	CTracker():m_trackLeftVisib(CColumn::kInvalidIndex){}
	virtual ~CTracker(){}

	//CSheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e) override;
	//CSheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e) override;
	//CSheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e) override;

	//CSheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e) override;

	//CSheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e) override;
	//CSheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e) override;

	//CSheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e) override;


	//CSheetState* OnTrackLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	//CSheetState* OnTrackLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	//CSheetState* OnTrackLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	//CSheetState* OnTrackRButtonDown(CSheet* pSheet, MouseEventArgs& e);
	//CSheetState* OnTrackMouseMove(CSheet* pSheet, MouseEventArgs& e);
	//CSheetState* OnTrackMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	//CSheetState* OnTrackSetCursor(CSheet* pSheet, SetCursorEventArgs& e);

public:

	void OnDividerDblClk(CSheet* pSheet, const MouseEvent& e) override
	{
		int idx = GetTrackLeftTopIndex(pSheet, e);
		if (idx != CBand::kInvalidIndex) {
			auto ptr = pSheet->Index2Pointer<TRC, VisTag>(idx);
			if (ptr) {
				pSheet->FitBandWidth<TRC>(ptr);
			}
		}
	}

	void OnSetCursor(CSheet* pSheet, const SetCursorEvent& e)
	{
		CPoint pt;
		::GetCursorPos(&pt);
		::ScreenToClient(pSheet->GetWndPtr()->m_hWnd, &pt);
		if (IsTarget(pSheet, MouseEvent(pSheet->GetWndPtr(), 0, MAKELPARAM(pt.x, pt.y)))) {
			*e.HandledPtr = TRUE;
			SetSizeCursor(); 
		} else {
			*e.HandledPtr = FALSE;
		}
	}

	void OnBeginTrack(CSheet* pSheet, const MouseEvent& e) override
	{
		m_trackLeftVisib = GetTrackLeftTopIndex(pSheet, e);
		SetSizeCursor();
	}

	void OnTrack(CSheet* pSheet, const MouseEvent& e) override
	{
		SetSizeCursor();
		auto p = pSheet->Index2Pointer<TRC, VisTag>(m_trackLeftVisib);
		p->SetLength(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient).Get<TRC::PointTag>() - p->GetStart(), true);
		pSheet->Track<TRC>(p);
	}

	void OnEndTrack(CSheet* pSheet, const MouseEvent& e) override
	{
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		if (auto p = pSheet->Index2Pointer<TRC, VisTag>(m_trackLeftVisib)) {
			p->SetLength(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient).Get<TRC::PointTag>() - p->GetStart(), true);
			pSheet->EndTrack<TRC>(p);
		}
	}

	void OnLeaveTrack(CSheet* pSheet, const MouseEvent& e) override
	{
		//TODO
		//Should Candel?
	}

	int GetTrackLeftTopIndex(CSheet* pSheet, const MouseEvent& e)
	{
		if (!pSheet->Visible()) {
			return CBand::kInvalidIndex;
		}
		CPointF ptDips = pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);
		auto visIndexes = pSheet->Point2Indexes(ptDips);
		int minIdx = 0;
		int maxIdx = pSheet->GetContainer<TRC, VisTag>().size() - 1;

		//If Header except Filter
		if (visIndexes.Get<TRC::Other::IndexesTag>() < pSheet->GetFrozenCount<TRC::Other>()) {
			if (visIndexes.Get<TRC::IndexesTag>() < minIdx) {
				//Out of Left	
				return CBand::kInvalidIndex;
			}
			else if (visIndexes.Get<TRC::IndexesTag>() > maxIdx) {
				//Out of Right
				if (ptDips.Get<TRC::PointTag>() < pSheet->GetContainer<TRC, VisTag>().back()->GetEnd() + CBand::kResizeAreaHarfWidth) {
					return visIndexes.Get<TRC::IndexesTag>() - 1;
				}
				else
				{
					return CBand::kInvalidIndex;
				}
			}
			else if (ptDips.Get<TRC::PointTag>() < pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC::IndexesTag>())->GetStart() + CBand::kResizeAreaHarfWidth) {
				return (std::max)(visIndexes.Get<TRC::IndexesTag>() - 1, minIdx);
			}
			else if (pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC::IndexesTag>())->GetEnd() - CBand::kResizeAreaHarfWidth < ptDips.Get<TRC::PointTag>()) {
				return (std::min)(visIndexes.Get<TRC::IndexesTag>(), maxIdx);
			}
			else {
				return CBand::kInvalidIndex;
			}
		}
		else {
			return CBand::kInvalidIndex;
		}


	}

	bool IsTarget(CSheet* pSheet, const MouseEvent& e) override
	{
		if (!pSheet->Visible()) {
			return false;
		}
		CPointF ptDips = pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);
		auto visIndexes = pSheet->Point2Indexes(ptDips);
		auto other = pSheet->Coordinate2Pointer<TRC::Other>(ptDips.Get<TRC::Other::PointTag>());
		int minIdx = 0;
		int maxIdx = (int)pSheet->GetContainer<TRC, VisTag>().size() - 1;

		//If Header except Filter
		if (other!=nullptr && other->IsTrackable()) {
			if (visIndexes.Get<TRC::IndexesTag>() < minIdx) {
				//Out of Left	
				return false;
			}
			else if (visIndexes.Get<TRC::IndexesTag>() > maxIdx) {
				//Out of Right
				if (ptDips.Get<TRC::PointTag>() < pSheet->GetContainer<TRC, VisTag>().back()->GetEnd() + CBand::kResizeAreaHarfWidth) {
					//m_trackLeftVisib = visIndexes.Get<TRC>() - 1;
					return true;
				}
				else
				{
					return false;
				}
			} else if (ptDips.Get<TRC::PointTag>() < pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC::IndexesTag>())->GetStart() + CBand::kResizeAreaHarfWidth) {
				//m_trackLeftVisib = (std::max)(visIndexes.Get<TRC>() - 1, minIdx);
				return true;
			} else if (pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC::IndexesTag>())->GetEnd() - CBand::kResizeAreaHarfWidth < ptDips.Get<TRC::PointTag>()) {
				//m_trackLeftVisib = (std::min)(visIndexes.Get<TRC>(), maxIdx);
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

private:
	void SetSizeCursor(){}

};

template<> inline void CTracker<RowTag>::SetSizeCursor()
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
}

template<> inline void CTracker<ColTag>::SetSizeCursor()
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
}