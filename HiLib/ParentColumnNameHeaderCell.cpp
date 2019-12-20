#include "ParentColumnNameHeaderCell.h"
#include "CellProperty.h"
#include "MyRect.h"
#include "MyColor.h"
#include "Sheet.h"
#include "Column.h"
#include "SheetEventArgs.h"

CMenu CParentColumnNameHeaderCell::ContextMenu;
const FLOAT CParentColumnNameHeaderCell::TRIANGLE_WIDTH = 12.f;
const FLOAT CParentColumnNameHeaderCell::TRIANGLE_HEIGHT = 6.f;
const d2dw::CRectF CParentColumnNameHeaderCell::TRIANGLE_MARGIN=d2dw::CRectF(2.f,0.f,2.f,0.f);
const FLOAT CParentColumnNameHeaderCell::MIN_COLUMN_WIDTH = 16.f;


Sorts CParentColumnNameHeaderCell::GetSort()const
{
	return m_pColumn->GetSort();
}

void CParentColumnNameHeaderCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	switch(GetSort()){
	case Sorts::Up:
	case Sorts::Down:
		{
			d2dw::CRectF rcText(rcPaint);
			rcText.right = (std::max)(rcText.left,  rcText.right - (TRIANGLE_WIDTH+TRIANGLE_MARGIN.left+TRIANGLE_MARGIN.right));
			CTextCell::PaintContent(pDirect,rcText);
			d2dw::CRectF rcSort(rcPaint);
			rcSort.left=rcText.right;
			PaintSortMark(pDirect,rcSort);
			break;
		}
	default:
		CTextCell::PaintContent(pDirect,rcPaint);
		break;
	}
}

void CParentColumnNameHeaderCell::PaintSortMark(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	std::array<d2dw::CPointF,3> arPoint;

	switch(GetSort()){
	case Sorts::Down:
		{
			FLOAT top=(rcPaint.Height()-TRIANGLE_HEIGHT)/2;
			arPoint[0]= d2dw::CPointF(TRIANGLE_MARGIN.left,top);
			arPoint[1]= d2dw::CPointF(TRIANGLE_MARGIN.left+TRIANGLE_WIDTH/2,top+TRIANGLE_HEIGHT);
			arPoint[2]= d2dw::CPointF(TRIANGLE_MARGIN.left+TRIANGLE_WIDTH,top);
			break;
		}
	case Sorts::Up:
		{
			FLOAT bottom=(rcPaint.Height()-TRIANGLE_HEIGHT)/2+TRIANGLE_HEIGHT;
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
	//TODO Fill Geometry
	d2dw::SolidLine line(0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	pDirect->DrawSolidLine(line, arPoint[0], arPoint[1]);
	pDirect->DrawSolidLine(line, arPoint[1], arPoint[2]);
	pDirect->DrawSolidLine(line, arPoint[2], arPoint[0]);
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

d2dw::CSizeF CParentColumnNameHeaderCell::MeasureContentSize(d2dw::CDirect2DWrite* pDirect)
{
	d2dw::CSizeF size = CTextCell::MeasureContentSize(pDirect);
	d2dw::CSizeF sizeTri = GetSortSize();

	size.width += sizeTri.width;
	size.height = (std::max)(size.height, sizeTri.height);

	return size;
}

d2dw::CSizeF CParentColumnNameHeaderCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	d2dw::CSizeF size = CTextCell::MeasureContentSizeWithFixedWidth(pDirect);
	d2dw::CSizeF sizeTri = GetSortSize();

	size.width += sizeTri.width;
	size.height = (std::max)(size.height, sizeTri.height);

	return size;
}

void CParentColumnNameHeaderCell::OnLButtonClk(const LButtonClkEvent& e)
{
	m_pSheet->CellLButtonClk(CellEventArgs(this));
}
