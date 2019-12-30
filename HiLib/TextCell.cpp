#include "TextCell.h"
#include "CellProperty.h"
#include "MyString.h"
#include "MyRect.h"
#include "MySize.h"
#include "MyDC.h"
#include "MyRgn.h"
#include "Sheet.h"
#include "Row.h"
#include "Column.h"
#include "GridView.h"
#include <algorithm>
#include "Textbox.h"


CTextCell::~CTextCell(){}

void CTextCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	pDirect->DrawTextLayout(*(m_spProperty->Format), GetString(), rcPaint);
}

d2dw::CSizeF CTextCell::MeasureContentSize(d2dw::CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return pDirect->CalcTextSize(*(m_spProperty->Format), text);
}

d2dw::CSizeF CTextCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	d2dw::CRectF rcCenter(0,0,m_pColumn->GetWidth(),0);
	d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));

	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return pDirect->CalcTextSizeWithFixedWidth(*(m_spProperty->Format), text, rcContent.Width());
}

d2dw::CRectF CTextCell::GetEditRect() const
{
	return GetRect();
}

void CTextCell::OnEdit(const EventArgs& e)
{
	m_pSheet->GetGridPtr()->BeginEdit(this);
}

void CTextCell::PaintLine(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	CCell::PaintLine(pDirect, rcPaint);

}

void CTextCell::PaintBackground(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	CCell::PaintBackground(pDirect, rcPaint);
}
	
bool CTextCell::IsComparable()const{return true;}

Compares CTextCell::EqualCell(CCell* pCell, std::function<void(CCell*, Compares)> action)
{
	return pCell->EqualCell(this, action);
}

Compares CTextCell::EqualCell(CEmptyCell* pCell, std::function<void(CCell*, Compares)> action)
{
	action(this, Compares::DiffNE);
	return Compares::DiffNE;
}

Compares CTextCell::EqualCell(CTextCell* pCell, std::function<void(CCell*, Compares)> action)
{
	auto str=GetString();
	bool bEqual=true;
	if(IsNaN(str)){
		bEqual= str==pCell->GetString();
	}else{
		bEqual= equal_wstring_compare_in_double()(str,pCell->GetString());
	}
	action(this, bEqual?Compares::Same:Compares::Diff);
	return bEqual?Compares::Same:Compares::Diff;
}

Compares CTextCell::EqualCell(CSheetCell* pCell, std::function<void(CCell*, Compares)> action)
{
	action(this, Compares::Diff);
	return Compares::Diff;
}

void CTextCell::OnKillFocus(const KillFocusEvent& e){}


std::wstring CStringCell::GetString()
{
	return m_string;
}

void CStringCell::SetStringCore(const std::wstring& str)
{
	m_string=str;
}


void CEditableCell::OnLButtonDown(const LButtonDownEvent& e)
{
	OnEdit(e);
}


void CEditableStringCell::OnLButtonDown(const LButtonDownEvent& e)
{
	OnEdit(e);
}

void CParameterCell::OnLButtonDown(const LButtonDownEvent& e)
{
	//Do Nothing
}
void CParameterCell::OnLButtonSnglClk(const LButtonSnglClkEvent& e)
{
	if(GetDoubleFocused()){
		OnEdit(e);
	}

}

void CParameterCell::OnKillFocus(const KillFocusEvent& e)
{
	m_bFirstFocus=false;
	CCell::OnKillFocus(e);
}