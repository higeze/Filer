#include "FavoriteRow.h"
#include "Sheet.h"

CFavoriteRow::CFavoriteRow(CGridView* pGrid, int orderIndex)
	:CParentRow(pGrid){}

int CFavoriteRow::GetOrderIndex()const { return GetIndex<VisTag>(); }
