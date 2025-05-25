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

void CTextCell::OnPropertyChanged(const wchar_t* name)
{
	if (!_tcsicmp(L"value", name)) {
		//Update valid flag
		m_isFitMeasureValid = false;
		m_isActMeasureValid = false;
		m_pTextLayout = nullptr;
	}
	else if (!_tcsicmp(L"size", name)) {
		m_isFitMeasureValid = false;
		m_isActMeasureValid = false;
		m_pTextLayout = nullptr;
	}
	//Notify to Row, Column and Sheet
	m_pRow->OnCellPropertyChanged(this, name);
	m_pColumn->OnCellPropertyChanged(this, name);
	m_pGrid->OnCellPropertyChanged(this, name);
}

const CComPtr<IDWriteTextLayout1>& CTextCell::GetTextLayoutPtr() const
{
	if (!m_pTextLayout && !GetString().empty()) {
		m_pTextLayout = GetTextLayoutPtrCore();
	}
	return m_pTextLayout;
}

const CComPtr<IDWriteTextLayout1> CTextCell::GetTextLayoutPtrCore() const
{
	auto pDirect = GetGridPtr()->GetWndPtr()->GetDirectPtr();
	auto pFactory = pDirect->GetDWriteFactory();
	auto size = TextRect().Size();

	CComPtr<IDWriteTextLayout> pTextLayout0(nullptr);
	CComPtr<IDWriteTextLayout1> pTextLayout1(nullptr);

	const IID* piid = &__uuidof(IDWriteTextLayout1);
	if (FAILED(pFactory->CreateTextLayout(GetString().c_str(), GetString().size(), pDirect->GetTextFormat(GetFormat()), size.width, size.height, &pTextLayout0)) ||
		FAILED(pTextLayout0->QueryInterface(&pTextLayout1))) {
		throw std::exception(FILE_LINE_FUNC);
	}
	else {
		//Default set up
		CComPtr<IDWriteTypography> typo;
		pFactory->CreateTypography(&typo);

		DWRITE_FONT_FEATURE feature;
		feature.nameTag = DWRITE_FONT_FEATURE_TAG_STANDARD_LIGATURES;
		feature.parameter = 0;
		typo->AddFontFeature(feature);
		DWRITE_TEXT_RANGE range;
		range.startPosition = 0;
		range.length = GetString().size();
		pTextLayout1->SetTypography(typo, range);

		pTextLayout1->SetCharacterSpacing(0.0f, 0.0f, 0.0f, DWRITE_TEXT_RANGE{ 0, GetString().size() });
		pTextLayout1->SetPairKerning(FALSE, DWRITE_TEXT_RANGE{ 0, GetString().size() });
	}
	return pTextLayout1;
}



void CTextCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	if (!GetString().empty()) {
		pDirect->GetD2DDeviceContext()->DrawTextLayout(
			rcPaint.LeftTop(), GetTextLayoutPtr(), pDirect->GetColorBrush(GetFormat().Color), D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);
	}

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
