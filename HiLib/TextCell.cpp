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


void CTextCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	pDirect->DrawTextLayout(*(m_spCellProperty->Format), GetString(), rcPaint);
}

d2dw::CSizeF CTextCell::MeasureContentSize(d2dw::CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return pDirect->CalcTextSize(*(m_spCellProperty->Format), text);
}

d2dw::CSizeF CTextCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	
	d2dw::CRectF rcCenter(0,0,m_pColumn->GetWidth(),0);
	d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));

	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return pDirect->CalcTextSizeWithFixedWidth(*(m_spCellProperty->Format), text, rcContent.Width());
}

d2dw::CRectF CTextCell::GetEditRect() const
{
	return GetRect();
}

void CTextCell::OnEdit(const EventArgs& e)
{
	if (GetIsVisible()) {
		m_pSheet->GetGridPtr()->BeginEdit(this);
	}
}

void CTextCell::PaintLine(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	CCell::PaintLine(pDirect, rcPaint);
}

void CTextCell::PaintBackground(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	CCell::PaintBackground(pDirect, rcPaint);
}

void CTextCell::OnLButtonDown(const LButtonDownEvent& e)
{
	if (m_editMode == EditMode::LButtonDownEdit) {
		OnEdit(e);
	}
	CCell::OnLButtonDown(e);
}

void CTextCell::OnLButtonDblClk(const LButtonDblClkEvent& e)
{
	if (m_editMode == EditMode::ExcelLike) {
		OnEdit(e);
	}
	CCell::OnLButtonDblClk(e);
}

void CTextCell::OnLButtonSnglClk(const LButtonSnglClkEvent& e)
{
	if (m_editMode == EditMode::FocusedSingleClickEdit) {
		if (GetIsDoubleFocused()) {
			OnEdit(e);
		}
	}
	CCell::OnLButtonSnglClk(e);
}

void CTextCell::OnKeyDown(const KeyDownEvent& e)
{
	switch (e.Char) {
	case VK_F2:
		OnEdit(e);
		break;
	}
	CCell::OnKeyDown(e);
}

void CTextCell::OnChar(const CharEvent& e)
{
	if (m_editMode == EditMode::ExcelLike) {
		if ((UINT)(L' ') < e.Char && e.Char < 256) {
			OnEdit(e);
			m_pSheet->GetGridPtr()->GetEditPtr()->OnChar(e);
		}
	}
	CCell::OnChar(e);
}

std::wstring CTextCell::GetString()
{
	return m_text;
}

void CTextCell::SetStringCore(const std::wstring& str)
{
	m_text = str;
}
