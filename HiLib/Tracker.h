#pragma once

#include "Column.h"

//Pre-Declaration
class CSheet;
struct MouseEventArgs;
struct SetCursorEventArgs;

class ITracker
{
public:
	virtual void OnDividerDblClk(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnBeginTrack(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnTrack(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnEndTrack(CSheet* pSheet, MouseEventArgs const & e) = 0;
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
		auto p = pSheet->Index2Pointer<ColTag, VisTag>(pSheet->Point2Coordinate<TRC>(e.Point));
		//pSheet->HeaderFitWidth(CColumnEventArgs(p.get()));//TODO
	}

	void OnBeginTrack(CSheet* pSheet, MouseEventArgs const & e) override
	{
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
		m_trackLeftVisib = pSheet->Coordinate2Index<TRC, VisTag>(pSheet->Point2Coordinate<TRC>(e.Point));
	}

	void OnTrack(CSheet* pSheet, MouseEventArgs const & e) override
	{
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
		auto p = pSheet->Index2Pointer<ColTag, VisTag>(m_trackLeftVisib);
		//p->SetWidthWithoutSignal(max(e.Point.x - pCol->GetLeft(), CColumn::kMinWidth));//TODO
		//pSheet->ColumnHeaderTrack(CColumnEventArgs(pCol.get()));//TODO
	}

	void OnEndTrack(CSheet* pSheet, MouseEventArgs const & e) override
	{
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		auto pCol = pSheet->Index2Pointer<ColTag, VisTag>(m_trackLeftVisib);
		//pCol->SetWidthWithoutSignal(max(e.Point.x - pCol->GetLeft(), CColumn::kMinWidth));//TODO
		//pSheet->ColumnHeaderEndTrack(CColumnEventArgs(pCol.get()));//TODO
	}
};
