#include "TextCell.h"
#include "CellProperty.h"
#include "MyString.h"
#include "MyRect.h"
#include "MySize.h"
#include "MyDC.h"
#include "MyRgn.h"
#include "GridView.h"
#include "Row.h"
#include "Column.h"
#include "GridView.h"
#include <algorithm>
#include "CellTextBox.h"


void CTextCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	pDirect->DrawTextLayout(GetFormat(), GetString(), rcPaint);
}

CSizeF CTextCell::MeasureContentSize(CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return pDirect->CalcTextSize(GetFormat(), text);
}

CSizeF CTextCell::MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	
	CRectF rcCenter(0,0,m_pColumn->GetWidth(),0);
	CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));

	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return pDirect->CalcTextSizeWithFixedWidth(GetFormat(), text, rcContent.Width());
}

CRectF CTextCell::GetEditRect() const
{
	return GetRectInWnd();
}

void CTextCell::OnEdit(const Event& e)
{
	if (GetIsVisible()) {
		m_pGrid->BeginEdit(this);
	}
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
	CCell::OnKeyDown(e);
}

void CTextCell::OnKeyTraceDown(const KeyTraceDownEvent& e)
{
	if (m_editMode == EditMode::ExcelLike &&
		(!IsKeyDown(VK_SHIFT) && !IsKeyDown(VK_CONTROL) && !IsKeyDown(VK_MENU)) &&
		(e.Char == VK_F2 ||
		(e.Char >= '0' && e.Char <= '9') ||
		(e.Char >='A' && e.Char <='Z'))) {
		OnEdit(e);
		*e.HandledPtr = FALSE;
	}
	CCell::OnKeyTraceDown(e);
}

void CTextCell::OnChar(const CharEvent& e)
{
	if(m_pGrid->GetEditPtr()){
		m_pGrid->GetEditPtr()->OnChar(e);
	}
	CCell::OnChar(e);
}

void CTextCell::OnImeStartComposition(const ImeStartCompositionEvent& e)
{
	if (m_editMode == EditMode::ExcelLike) {
		OnEdit(e);
		*e.HandledPtr = FALSE;
	}
}

//
//std::wstring CTextCell::GetString()
//{
//	return m_text;
//}
//
//void CTextCell::SetStringCore(const std::wstring& str)
//{
//	m_text = str;
//}
