#pragma once
#include "D2DWTypes.h"
#include "reactive_property.h"
#include "reactive_string.h"

class CTextCaret
{
public:
	reactive_property_ptr<int> Old;
	reactive_property_ptr<int> Current;
	reactive_property_ptr<int> Anchor;
	reactive_property_ptr<int> SelectedBegin;
	reactive_property_ptr<int> SelectedEnd;

	reactive_property_ptr<CPointF> Point;

	CTextCaret(int old = 0, int current = 0, int anchor = 0, int selectedBegin = 0, int selectedEnd = 0, const CPointF& point = CPointF())
		:Old(old),
		Current(current),
		Anchor(anchor),
		SelectedBegin(selectedBegin),
		SelectedEnd(selectedEnd),
		Point(point){}

	bool operator==(const CTextCaret& rhs) const = default;

	void Set(int old = 0, int current = 0, int anchor = 0, int selectedBegin = 0, int selectedEnd = 0, const CPointF& point = CPointF())
	{
		Old.set(old);
		Current.set(current);
		Anchor.set(anchor);
		SelectedBegin.set(selectedBegin);
		SelectedEnd.set(selectedEnd);
		Point.set(point);
	}
	CTextCaret Clone() const;
	void CopyValueOnly(const CTextCaret& rhs);

	void Clear();
	void Move(const int& index, const CPointF& point);
	void MoveWithShift(const int& index, const CPointF& point);
	void Select(const int& selectedBegin, const int& selectedEnd, const CPointF& point);

	bool IsNotSelected() const { return *SelectedBegin == *SelectedEnd; }
};
