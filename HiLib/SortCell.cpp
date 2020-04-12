#include "SortCell.h"
#include "CellProperty.h"
#include "MyRect.h"
#include "MyColor.h"
#include "Sheet.h"
#include "Column.h"
#include "SheetEventArgs.h"

const FLOAT CSortCell::TRIANGLE_WIDTH = 12.f;
const FLOAT CSortCell::TRIANGLE_HEIGHT = 6.f;
const d2dw::CRectF CSortCell::TRIANGLE_MARGIN=d2dw::CRectF(2.f,0.f,2.f,0.f);
const FLOAT CSortCell::MIN_COLUMN_WIDTH = 16.f;


void CSortCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	switch(m_pColumn->GetSort()){
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

void CSortCell::PaintSortMark(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	std::array<d2dw::CPointF,3> arPoint;

	switch(m_pColumn->GetSort()){
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

d2dw::CSizeF CSortCell::GetSortSize()const
{
	switch(m_pColumn->GetSort()){
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

d2dw::CSizeF CSortCell::MeasureContentSize(d2dw::CDirect2DWrite* pDirect)
{
	d2dw::CSizeF size = CTextCell::MeasureContentSize(pDirect);
	d2dw::CSizeF sizeTri = GetSortSize();

	size.width += sizeTri.width;
	size.height = (std::max)(size.height, sizeTri.height);

	return size;
}

d2dw::CSizeF CSortCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	d2dw::CSizeF size = CTextCell::MeasureContentSizeWithFixedWidth(pDirect);
	d2dw::CSizeF sizeTri = GetSortSize();

	size.width += sizeTri.width;
	size.height = (std::max)(size.height, sizeTri.height);

	return size;
}

void CSortCell::OnLButtonClk(const LButtonClkEvent& e)
{
	//Get current Sort before all reset 
	auto sort = m_pColumn->GetSort();
	//Reset All Sort
	m_pSheet->ResetColumnSort();
	//Sort
	switch(sort){
		case Sorts::None:
		case Sorts::Down:
			m_pColumn->SetSort(Sorts::Up);
			break;
		case Sorts::Up:
			m_pColumn->SetSort(Sorts::Down);
			break;
		default:
			m_pColumn->SetSort(Sorts::None);
			break;
	}
	m_pSheet->SubmitUpdate();
}
