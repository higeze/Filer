#pragma once
#include "Sheet.h"
#include "Column.h"

//Pre-Declaration
class CSheet;
struct RowTag;
struct ColTag;
struct MouseEventArgs;
struct SetCursorEventArgs;

class ITracker
{
public:
	virtual void OnDividerDblClk(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnBeginTrack(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnTrack(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnEndTrack(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnLeaveTrack(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnSetCursor(CSheet* pSheet, SetCursorEventArgs const & e) = 0;
	virtual bool IsTarget(CSheet* pSheet, MouseEventArgs const & e) = 0;
};

template<typename TRC>
class CTracker:public ITracker
{
private:
	typedef int size_type;
	typedef int coordinates_type;
	//static const size_type COLUMN_INDEX_INVALID = 9999;
	//static const size_type COLUMN_INDEX_MAX = 1000;
	//static const size_type COLUMN_INDEX_MIN = -1000;
	//static const coordinates_type MIN_COLUMN_WIDTH=2;
	//static const coordinates_type RESIZE_AREA_HARF_WIDTH=4;

private:
	size_type m_trackLeftVisib;
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

	void OnDividerDblClk(CSheet* pSheet, MouseEventArgs const & e) override
	{
		auto idx = GetTrackLeftTopIndex(pSheet, e);
		if (idx != CBand::kInvalidIndex) {
			auto ptr = pSheet->Index2Pointer<TRC, VisTag>(idx);
			if (ptr) {
				pSheet->FitBandWidth<TRC>(ptr);
			}
		}
	}

	void OnSetCursor(CSheet* pSheet, SetCursorEventArgs const & e)
	{
		CPoint pt;
		::GetCursorPos(&pt);
		::ScreenToClient(e.HWnd, &pt);
		if (IsTarget(pSheet, MouseEventArgs(NULL, pt))) {
			e.Handled = TRUE;
			SetSizeCursor(); 
		}
	}

	void OnBeginTrack(CSheet* pSheet, MouseEventArgs const & e) override
	{
		//e.Handled = TRUE;
		m_trackLeftVisib = GetTrackLeftTopIndex(pSheet, e);
		SetSizeCursor();
	}

	void OnTrack(CSheet* pSheet, MouseEventArgs const & e) override
	{
		//e.Handled = TRUE;
		SetSizeCursor();
		auto p = pSheet->Index2Pointer<TRC, VisTag>(m_trackLeftVisib);
		p->SetWidthHeightWithoutSignal((std::max)(e.Point.Get<TRC::Axis>() - p->GetLeftTop(), (LONG)CColumn::kMinWidth));//TODO
		pSheet->Track<TRC>(p);
	}

	void OnEndTrack(CSheet* pSheet, MouseEventArgs const & e) override
	{
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		auto p = pSheet->Index2Pointer<TRC, VisTag>(m_trackLeftVisib);
		p->SetWidthHeightWithoutSignal((std::max)(e.Point.Get<TRC::Axis>() - p->GetLeftTop(), (LONG)CColumn::kMinWidth));//TODO
		pSheet->EndTrack<TRC>(p);
	}

	void OnLeaveTrack(CSheet* pSheet, MouseEventArgs const & e) override
	{
		//TODO
		//Should Candel?
	}

	int GetTrackLeftTopIndex(CSheet* pSheet, MouseEventArgs const & e)
	{
		if (!pSheet->Visible()) {
			return CBand::kInvalidIndex;
		}
		auto visIndexes = pSheet->Point2Indexes<VisTag>(e.Point);
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
				if (e.Point.Get<TRC::Axis>() < pSheet->LastPointer<TRC, VisTag>()->GetRightBottom() + CBand::kResizeAreaHarfWidth) {
					return visIndexes.Get<TRC>() - 1;
				}
				else
				{
					return CBand::kInvalidIndex;
				}
			}
			else if (e.Point.Get<TRC::Axis>() < (pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC>())->GetLeftTop() + CBand::kResizeAreaHarfWidth)) {
				return (std::max)(visIndexes.Get<TRC>() - 1, minIdx);
			}
			else if ((pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC>())->GetRightBottom() - CBand::kResizeAreaHarfWidth) < e.Point.Get<TRC::Axis>()) {
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

	bool IsTarget(CSheet* pSheet, MouseEventArgs const & e) override
	{
		if (!pSheet->Visible()) {
			return false;
		}
		auto visIndexes = pSheet->Point2Indexes<VisTag>(e.Point);
		auto minIdx = pSheet->GetMinIndex<TRC, VisTag>();
		auto maxIdx = pSheet->GetMaxIndex<TRC, VisTag>();

		//If Header except Filter
		if (visIndexes.Get<TRC::Other>() < 0) {
			if (visIndexes.Get<TRC>() < minIdx) {
				//Out of Left	
				return false;
			}
			else if (visIndexes.Get<TRC>() > maxIdx) {
				//Out of Right
				if (e.Point.Get<TRC::Axis>() < pSheet->LastPointer<TRC, VisTag>()->GetRightBottom() + CBand::kResizeAreaHarfWidth) {
					//m_trackLeftVisib = visIndexes.Get<TRC>() - 1;
					return true;
				}
				else
				{
					return false;
				}
			}
			else if (e.Point.Get<TRC::Axis>() < (pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC>())->GetLeftTop() + CBand::kResizeAreaHarfWidth)) {
				//m_trackLeftVisib = (std::max)(visIndexes.Get<TRC>() - 1, minIdx);
				return true;
			}
			else if ((pSheet->Index2Pointer<TRC, VisTag>(visIndexes.Get<TRC>())->GetRightBottom() - CBand::kResizeAreaHarfWidth) < e.Point.Get<TRC::Axis>()) {
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