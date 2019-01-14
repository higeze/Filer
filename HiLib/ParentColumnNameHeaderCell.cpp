#include "ParentColumnNameHeaderCell.h"
#include "CellProperty.h"
#include "MyRect.h"
#include "MyColor.h"
#include "Sheet.h"
#include "Column.h"
#include "SheetEventArgs.h"

CMenu CParentColumnNameHeaderCell::ContextMenu;
const CRect CParentColumnNameHeaderCell::TRIANGLE_MARGIN=CRect(2,0,2,0);

Sorts CParentColumnNameHeaderCell::GetSort()const
{
	return m_pColumn->GetSort();
}

void CParentColumnNameHeaderCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	switch(GetSort()){
	case Sorts::Up:
	case Sorts::Down:
		{
			d2dw::CRectF rcText(rcPaint);
			rcText.right-=TRIANGLE_WIDTH+TRIANGLE_MARGIN.left+TRIANGLE_MARGIN.right;
			CTextCell::PaintContent(direct,rcText);
			d2dw::CRectF rcSort(rcPaint);
			rcSort.left=rcText.right;
			PaintSortMark(direct,rcSort);
			break;
		}
	default:
		CTextCell::PaintContent(direct,rcPaint);
		break;
	}
}

void CParentColumnNameHeaderCell::PaintSortMark(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	std::array<d2dw::CPointF,3> arPoint;

	switch(GetSort()){
	case Sorts::Down:
		{
			int top=(rcPaint.Height()-TRIANGLE_HEIGHT)/2;
			arPoint[0]= d2dw::CPointF(TRIANGLE_MARGIN.left,top);
			arPoint[1]= d2dw::CPointF(TRIANGLE_MARGIN.left+TRIANGLE_WIDTH/2,top+TRIANGLE_HEIGHT);
			arPoint[2]= d2dw::CPointF(TRIANGLE_MARGIN.left+TRIANGLE_WIDTH,top);
			break;
		}
	case Sorts::Up:
		{
			int bottom=(rcPaint.Height()-TRIANGLE_HEIGHT)/2+TRIANGLE_HEIGHT;
			arPoint[0]= d2dw::CPointF(TRIANGLE_MARGIN.left,bottom);
			arPoint[1]= d2dw::CPointF(TRIANGLE_MARGIN.left+TRIANGLE_WIDTH/2,bottom-TRIANGLE_HEIGHT);
			arPoint[2]= d2dw::CPointF(TRIANGLE_MARGIN.left+TRIANGLE_WIDTH,bottom);
			break;
		}

	default:
		return;
		break;
	}
	d2dw::CPointF ptTopRight=rcPaint.LeftTop();
	for(auto iter=arPoint.begin(),end=arPoint.end();iter!=end;++iter){
		*iter+=ptTopRight;
	}
	//TODOTODO
	//HPEN hPen=pDC->SelectPen((HPEN)::GetStockObject(WHITE_PEN));
	//HBRUSH hBr = pDC->SelectBrush((HBRUSH)::GetStockObject(LTGRAY_BRUSH));
	//::Polygon(*pDC,&arPoint[0],3);
	//pDC->SelectPen(hPen);
	//pDC->SelectBrush(hBr);

}

d2dw::CSizeF CParentColumnNameHeaderCell::GetSortSize()const
{
	switch(GetSort()){
	case Sorts::Up:
	case Sorts::Down:
		return d2dw::CSizeF(TRIANGLE_WIDTH+TRIANGLE_MARGIN.left+TRIANGLE_MARGIN.right,
					TRIANGLE_HEIGHT+TRIANGLE_MARGIN.top+TRIANGLE_MARGIN.bottom);
		break;
	default:
		return d2dw::CSizeF();
		break;
	}

}

d2dw::CSizeF CParentColumnNameHeaderCell::MeasureContentSize(d2dw::CDirect2DWrite& direct)
{
	d2dw::CSizeF size = CTextCell::MeasureContentSize(direct);
	d2dw::CSizeF sizeTri = GetSortSize();

	size.width += sizeTri.width;
	size.height = (std::max)(size.height, sizeTri.height);

	return size;
}

d2dw::CSizeF CParentColumnNameHeaderCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	d2dw::CSizeF size = CTextCell::MeasureContentSizeWithFixedWidth(direct);
	d2dw::CSizeF sizeTri = GetSortSize();

	size.width += sizeTri.width;
	size.height = (std::max)(size.height, sizeTri.height);

	return size;
}

void CParentColumnNameHeaderCell::OnLButtonClk(const LButtonClkEvent& e)
{
	m_pSheet->CellLButtonClk(CellEventArgs(this));
}
