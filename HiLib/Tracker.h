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
		auto idx = GetTrackLeftTopIndex(pSheet, e);
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
		::ScreenToClient(e.WindowPtr->m_hWnd, &pt);
		if (IsTarget(pSheet, MouseEvent(e.WindowPtr, *(pSheet->GetGridPtr()->GetDirect()), 0,  pt))) {
			e.Handled = TRUE;
			SetSizeCursor(); 
		}
	}

	void OnBeginTrack(CSheet* pSheet, const MouseEvent& e) override
	{
		//e.Handled = TRUE;
		m_trackLeftVisib = GetTrackLeftTopIndex(pSheet, e);
		SetSizeCursor();
	}

	void OnTrack(CSheet* pSheet, const MouseEvent& e) override
	{
		//e.Handled = TRUE;
		SetSizeCursor();
		auto p = pSheet->Index2Pointer<TRC, VisTag>(m_trackLeftVisib);
		p->SetWidthHeightWithoutSignal(e.Direct.Pixels2Dips(e.Point).Get<TRC::Axis>() - p->GetLeftTop());
		pSheet->Track<TRC>(p);
	}

	void OnEndTrack(CSheet* pSheet, const MouseEvent& e) override
	{
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		auto p = pSheet->Index2Pointer<TRC, VisTag>(m_trackLeftVisib);
		p->SetWidthHeightWithoutSignal(e.Direct.Pixels2Dips(e.Point).Get<TRC::Axis>() - p->GetLeftTop());
		pSheet->EndTrack<TRC>(p);
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
		d2dw::CPointF ptDips = e.Direct.Pixels2Dips(e.Point);
		auto visIndexes = pSheet->Point2Indexes<VisTag>(ptDips);
		auto minIdx = pSheet->GetMinIndex<TRC, VisTag>();
		auto maxIdx = pSheet->GetMaxIndex<TRC, VisTag>();

		//If Header except Filter
		if (visIndexes.Get<TRC::Other>() < 0) {
			if (visIndexes.Get<TRC>() < minIdx) {
				//Out of Left	
				return CBand::kInvalidIndex;
			}
			else if (visIndexes.Get<TRC>() > maxIdx) {
				//Out of Right
				if (ptDips.Get<TRC::Axis>() < pSheet->LastPointer<TRC, VisTag>()->GetRightBottom() + CBand::kResizeAreaHarfWidth) {
					return visIndexes.Get<TRC>() - 1;
				}
				else
				{
					return CBand::kInvalidIndex;
				}
			}
			else if (ptDips.Get<TRC::Axis>() < (pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC>())->GetLeftTop() + CBand::kResizeAreaHarfWidth)) {
				return (std::max)(visIndexes.Get<TRC>() - 1, minIdx);
			}
			else if ((pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC>())->GetRightBottom() - CBand::kResizeAreaHarfWidth) < ptDips.Get<TRC::Axis>()) {
				return (std::min)(visIndexes.Get<TRC>(), maxIdx);
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
		d2dw::CPointF ptDips = e.Direct.Pixels2Dips(e.Point);
		auto visIndexes = pSheet->Point2Indexes<VisTag>(ptDips);
		auto other = pSheet->Coordinate2Pointer<TRC::Other>(ptDips.Get<TRC::OtherAxis>());
		auto minIdx = pSheet->GetMinIndex<TRC, VisTag>();
		auto maxIdx = pSheet->GetMaxIndex<TRC, VisTag>();

		//If Header except Filter
		if (visIndexes.Get<TRC::Other>() < 0 && (other==nullptr || other->IsTrackable())) {
			if (visIndexes.Get<TRC>() < minIdx) {
				//Out of Left	
				return false;
			}
			else if (visIndexes.Get<TRC>() > maxIdx) {
				//Out of Right
				if (ptDips.Get<TRC::Axis>() < pSheet->LastPointer<TRC, VisTag>()->GetRightBottom() + CBand::kResizeAreaHarfWidth) {
					//m_trackLeftVisib = visIndexes.Get<TRC>() - 1;
					return true;
				}
				else
				{
					return false;
				}
			}
			else if (ptDips.Get<TRC::Axis>() < (pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC>())->GetLeftTop() + CBand::kResizeAreaHarfWidth)) {
				//m_trackLeftVisib = (std::max)(visIndexes.Get<TRC>() - 1, minIdx);
				return true;
			}
			else if ((pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC>())->GetRightBottom() - CBand::kResizeAreaHarfWidth) < ptDips.Get<TRC::Axis>()) {
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