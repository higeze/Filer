#include "TextCaret.h"

CTextCaret CTextCaret::Clone() const
{
	return CTextCaret(*Old, *Current, *Anchor, *SelectedBegin, *SelectedEnd);
}

void CTextCaret::CopyValueOnly(const CTextCaret& rhs)
{
	Old.set(*rhs.Old);
	Current.set(*rhs.Current);
	Anchor.set(*rhs.Anchor);
	SelectedBegin.set(*rhs.SelectedBegin);
	SelectedEnd.set(*rhs.SelectedEnd);
	Point.set(*rhs.Point);
}


void CTextCaret::Clear()
{
	Old.set(0);
	Current.set(0);
	Anchor.set(0);
	SelectedBegin.set(0);
	SelectedEnd.set(0);
	Point.set(CPointF());
}


void CTextCaret::Move(const int& index, const CPointF& point)
{
	Old.set(*Current);
	Current.set(index);
	Anchor.set(index);
	SelectedBegin.set(index);
	SelectedEnd.set(index);
	Point.set(point);
}

void CTextCaret::MoveWithShift(const int& index, const CPointF& point)
{
	Old.set(*Current);
	Current.set(index);
	Anchor.set(*Anchor);
	SelectedBegin.set((std::min)(*Anchor, index));
	SelectedEnd.set((std::max)(*Anchor, index));
	Point.set(point);
}

void CTextCaret::Select(const int& selectedBegin, const int& selectedEnd, const CPointF& point)
{
	Old.set(*Current);
	Current.set(selectedEnd);
	Anchor.set(selectedBegin);
	SelectedBegin.set(selectedBegin);
	SelectedEnd.set(selectedEnd);
	Point.set(point);
}