#include "TextCell.h"
#include "CellProperty.h"
#include "MyString.h"
#include "MyRect.h"
#include "MySize.h"
#include "MyDC.h"
#include "MyRgn.h"

#include "InplaceEdit.h"
#include "Sheet.h"
#include "Row.h"
#include "Column.h"
#include "GridView.h"
#include <algorithm>

CTextCell::~CTextCell()
{
	if(m_pEdit){
		m_pEdit->SendMessage(WM_CLOSE,0,0);
		m_pEdit = nullptr;
	}
}

void CTextCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	direct.DrawTextLayout(*(m_spProperty->FontAndColor), GetString(), rcPaint);
}

//CSize CStringSizeCalculater::CalcSize(const std::wstring& str)
//{
//	CSize ret(0, 0);
//	for (const auto& ch : str) {
//		const auto& iter = m_charMap.find(ch);
//		CSize sizeCh(0, 0);
//		if (iter != m_charMap.end()) {
//			sizeCh = iter->second;
//		} else {
//			HFONT hFont = (HFONT)m_pDC->SelectFont(m_pFont->operator HFONT());
//			CRect rcCh;
//			m_pDC->DrawTextExW(const_cast<LPWSTR>(&ch), 1, rcCh,
//				DT_CALCRECT | m_format & ~DT_WORDBREAK, NULL);
//			m_pDC->SelectFont(hFont);
//			sizeCh = rcCh.Size();
//			m_charMap.emplace(ch, sizeCh);
//		}
//		ret.cx += sizeCh.cx;
//		ret.cy = (std::max)(ret.cy, sizeCh.cy);
//	}
//
//	return ret;
//}
//
//CSize CStringSizeCalculater::CalcSizeWithFixedWidth(const std::wstring& str, const LONG& width)
//{
//	std::vector<CSize> ret;
//	ret.emplace_back(0, 0);
//	size_t i = 0;
//
//	for (const auto& ch : str) {
//		const auto& iter = m_charMap.find(ch);
//		CSize sizeCh(0, 0);
//		if (iter != m_charMap.end()) {
//			sizeCh = iter->second;
//		} else {
//			HFONT hFont = (HFONT)m_pDC->SelectFont(m_pFont->operator HFONT());
//			CRect rcCh;
//			m_pDC->DrawTextExW(const_cast<LPWSTR>(&ch), 1, rcCh,
//				DT_CALCRECT | m_format & ~DT_WORDBREAK, NULL);
//			m_pDC->SelectFont(hFont);
//			sizeCh = rcCh.Size();
//			m_charMap.emplace(ch, sizeCh);
//		}
//		if (ret.back().cx + sizeCh.cx > width) {
//			ret.push_back(sizeCh);
//		} else {
//			ret.back().cx += sizeCh.cx;
//			ret.back().cy = (std::max)(ret.back().cy, sizeCh.cy);
//		}
//	}
//
//	return CSize(width, std::accumulate(ret.begin(), ret.end(), 0L, [](LONG y, const CSize& rh)->LONG {return y + rh.cy; }));
//}


d2dw::CSizeF CTextCell::MeasureContentSize(d2dw::CDirect2DWrite& direct)
{
	//Calc Content Rect
	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return direct.CalcTextSize(m_spProperty->FontAndColor->Font, text);
}

//CSize CTextCell::MeasureSize(CDC* pDC)
//{
//	CRect rcContent(MeasureContentSize(pDC));
//
//	//Calc CenterBorder Rect
//	CRect rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
//	m_bFitMeasureValid = true;
//	return rcCenter.Size();	
//}

d2dw::CSizeF CTextCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	//Calc Content Rect
	d2dw::CRectF rcCenter(0,0,m_pColumn->GetWidth(),0);
	d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));

	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return direct.CalcTextSizeWithFixedWidth(m_spProperty->FontAndColor->Font, text, rcContent.Width());
}

void CTextCell::OnEdit(const EventArgs& e)
{
	CRect rc(m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(CenterBorder2InnerBorder(GetRect())));
	auto spCell = std::static_pointer_cast<CTextCell>(CSheet::Cell(m_pRow, m_pColumn));
	SetState(UIElementState::Hot);//During Editing, Keep Hot
	m_pEdit = new CInplaceEdit(
		m_pSheet->GetGridPtr(),
		[spCell]() -> std::basic_string<TCHAR>{
			return spCell->GetString();
		},
		[spCell](const std::basic_string<TCHAR>& str) -> void{
			spCell->SetString(str);
		},
		[spCell](const std::basic_string<TCHAR>& str) -> void{
			if(spCell->CanSetStringOnEditing()){
				spCell->SetString(str);
			}
		},
		[spCell]()->void{
			spCell->m_pEdit=NULL;
			spCell->SetState(UIElementState::Normal);//After Editing, Change Normal
		},
		m_spProperty->FontAndColor->Font.GetGDIFont(),
		GetFormat());

	m_pEdit->Create(m_pSheet->GetGridPtr()->m_hWnd, rc);
	m_pEdit->SetWindowText(GetString().c_str());
	CRect rcRect(m_pEdit->GetRect());
	CRect rcPadding(m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(*(m_spProperty->Padding)));
	rcRect.DeflateRect(rcPadding);
	m_pEdit->SetRect(rcRect);
	//m_pEdit->SetMargins(m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(*(m_spProperty->Padding)).left, m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(*(m_spProperty->Padding)).right);
	m_pEdit->SetFocus();
	m_pEdit->SetSel(0,-1);
	m_pEdit->ShowWindow(SW_SHOW);
}

void CTextCell::PaintBackground(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	if(m_pEdit){
		d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(GetRect())));
		d2dw::CRectF rcCurEdit(direct.Pixels2Dips(m_pSheet->GetGridPtr()->ScreenToClientRect(m_pEdit->GetWindowRect())));
		//m_pSheet->GetGridPtr()->SetEditRect(rcContent);
		if(rcContent!=rcCurEdit){
			m_pEdit->MoveWindow(direct.Dips2Pixels(rcContent),FALSE);
		}
	}
	CCell::PaintBackground(direct, rcPaint);
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
	//if(!m_bFirstFocus){
	//	m_bFirstFocus=true;
	//	return;
	//}else{
	//	CEditableCell::OnLButtonDown(e);
	//}

	if(GetDoubleFocused()){
		OnEdit(e);
	}

}

void CParameterCell::OnKillFocus(const KillFocusEvent& e)
{
	m_bFirstFocus=false;
	CCell::OnKillFocus(e);
}

//void CParameterCell::OnEdit(EventArgs& e)
//{
//	CRect rcContent(InnerBorder2Content(CenterBorder2InnerBorder(GetRect())));
//	auto spCell = CSheet::Cell(m_pRow, m_pColumn);
//	SetState(UIElementState::Hot);//During Editing, Keep Hot
//	m_pEdit = new CInplaceEdit(
//		[spCell]() -> std::basic_string<TCHAR>{
//			return spCell->GetString();
//		},
//		[spCell](const std::basic_string<TCHAR>& str) -> void{
//			spCell->SetString(str);
//		},
//		[spCell](const std::basic_string<TCHAR>& str) -> void{
//			//SetString(str);
//		},
//		[spCell]()->void{
//			std::dynamic_pointer_cast<CParameterCell>(spCell)->m_pEdit=NULL;
//			spCell->SetState(UIElementState::Normal);//After Editing, Change Normal
//		},
//		m_spProperty->GetDrawTextPropertyPtr());
//
//	m_pEdit->Create(m_pSheet->GetGridPtr()->m_hWnd,rcContent);
//	m_pEdit->SetWindowText(GetString().c_str());
//	m_pEdit->SetFont(*m_spProperty->GetFontPtr());
//	//CRect rcRect(m_pEdit->GetRect());
//	//CRect rcPadding(m_spProperty->GetPadding());
//	//rcRect.DeflateRect(rcPadding);
//	//m_pEdit->SetRect(rcRect);
//	//m_pEdit->SetMargins(2,2);
//	m_pEdit->SetFocus();
//	m_pEdit->SetSel(0,-1);
//	m_pEdit->ShowWindow(SW_SHOW);
//}