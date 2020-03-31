#include "FavoriteRow.h"
#include "Sheet.h"

CFavoriteRow::CFavoriteRow(CSheet* pSheet, int orderIndex)
	:CRow(pSheet){}

int CFavoriteRow::GetOrderIndex()const { return GetIndex<VisTag>(); }
