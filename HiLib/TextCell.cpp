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
#include "TextboxWnd.h"


CTextCell::~CTextCell()
{
	if(m_pEdit){
		SendMessage(m_pEdit->m_hWnd,WM_CLOSE,0,0);
		m_pEdit = nullptr;
	}
}

void CTextCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	direct.DrawTextLayout(*(m_spProperty->Format), GetString(), rcPaint);
}

d2dw::CSizeF CTextCell::MeasureContentSize(d2dw::CDirect2DWrite& direct)
{
	//Calc Content Rect
	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return direct.CalcTextSize(*(m_spProperty->Format), text);
}

d2dw::CSizeF CTextCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	//Calc Content Rect
	d2dw::CRectF rcCenter(0,0,m_pColumn->GetWidth(),0);
	d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));

	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	return direct.CalcTextSizeWithFixedWidth(*(m_spProperty->Format), text, rcContent.Width());
}

void CTextCell::OnEdit(const EventArgs& e)
{
	CRect rcEdit(m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(GetRect()));
	auto spCell = std::static_pointer_cast<CTextCell>(CSheet::Cell(m_pRow, m_pColumn));

	m_pEdit = new CTextboxWnd(
			m_spProperty,
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
			}
	);
	m_pEdit->Create(e.WindowPtr->m_hWnd, rcEdit);
}

void CTextCell::PaintBackground(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	if(m_pEdit){
		CRect rcCell(direct.Dips2Pixels(GetRect()));
		CRect rcEdit; ::GetWindowRect(m_pEdit->m_hWnd, &rcEdit);
		rcEdit = m_pSheet->GetGridPtr()->ScreenToClientRect(rcEdit);
		if(rcCell!=rcEdit){
			//::MoveWindow(m_pEdit->m_hWnd, rcCell.left, rcCell.top, rcCell.Width(), rcCell.Height(), FALSE);
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
	if(GetDoubleFocused()){
		OnEdit(e);
	}

}

void CParameterCell::OnKillFocus(const KillFocusEvent& e)
{
	m_bFirstFocus=false;
	CCell::OnKillFocus(e);
}