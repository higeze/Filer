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
#include "D2DWindow.h"
#include "D2DWindowControl.h"


CTextCell::~CTextCell()
{
	if(m_pEdit){
		SendMessage(m_pEdit->hWnd_,WM_CLOSE,0,0);
		m_pEdit = nullptr;
	}
}

void CTextCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	direct.DrawTextLayout(*(m_spProperty->Format), GetString(), rcPaint);
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
	return direct.CalcTextSize(*(m_spProperty->Format), text);
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
	return direct.CalcTextSizeWithFixedWidth(*(m_spProperty->Format), text, rcContent.Width());
}

#define IDLE_TIME (5*1000)
#define IDLE_TIMER_ID 99
#define IDB_PNG1	131

struct D2DMainFrame
{
	CComPtr<IDWriteFactory> wrfactory;
	CComPtr<ID2D1Factory>  factory;
	CComPtr<IDWriteTextFormat> textformat;
	CComPtr<ID2D1HwndRenderTarget> cxt;

	CComPtr<ID2D1SolidColorBrush> br[4];
	CComPtr<ID2D1SolidColorBrush> black, white;

	//FRectF btn[3];
	UINT btnStat;
	LPCWSTR title;

	enum COLORS { MOUSE_FLOAT, CLOSEBTN, ACTIVECAPTION, CLOSE_MOUSE_FLOAT };
};

//static D2DMainFrame __s_d2dmainframe = { 0 };
//void D2DInitial(HWND hWnd1, std::shared_ptr<d2dw::CDirect2DWrite>& pDirect, std::shared_ptr<CellProperty>& pProp)
//{
//	if (__s_d2dmainframe.cxt == nullptr) {
//		HRESULT hr;
//		//D2D1_FACTORY_OPTIONS options;
//		//options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
//		//hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &options, (void**)&__s_d2dmainframe.factory);
//		//_ASSERT(hr == S_OK);
//
//		//hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&__s_d2dmainframe.wrfactory));
//		//_ASSERT(hr == S_OK);
//
//		__s_d2dmainframe.textformat = pDirect->GetTextFormat(*(pProp->Format));
//
//		//hr = __s_d2dmainframe.wrfactory->CreateTextFormat(L"Arial", 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14, L"", &__s_d2dmainframe.textformat);
//		//_ASSERT(hr == S_OK);
//
//		__s_d2dmainframe.cxt = pDirect->GetHwndRenderTarget();
//		//hr = __s_d2dmainframe.factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd1, D2D1::SizeU(1, 1), D2D1_PRESENT_OPTIONS_NONE), &__s_d2dmainframe.cxt);
//		//_ASSERT(hr == S_OK);
//
//		__s_d2dmainframe.black = pDirect->GetColorBrush(d2dw::CColorF(0.f, 0.f, 0.f, 1.f));
//		__s_d2dmainframe.white = pDirect->GetColorBrush(d2dw::CColorF(1.f, 1.f, 1.f, 1.f));
//		//__s_d2dmainframe.cxt->CreateSolidColorBrush(ColorF(ColorF::Black), &__s_d2dmainframe.black);
//		//__s_d2dmainframe.cxt->CreateSolidColorBrush(ColorF(ColorF::White), &__s_d2dmainframe.white);
//		__s_d2dmainframe.cxt->CreateSolidColorBrush(D2RGBA(54, 101, 179, 255), &__s_d2dmainframe.br[D2DMainFrame::MOUSE_FLOAT]);
//		__s_d2dmainframe.cxt->CreateSolidColorBrush(D2RGBA(199, 80, 80, 255), &__s_d2dmainframe.br[D2DMainFrame::CLOSEBTN]);
//		__s_d2dmainframe.cxt->CreateSolidColorBrush(D2RGBA(144, 169, 184, 255), &__s_d2dmainframe.br[D2DMainFrame::ACTIVECAPTION]);
//		__s_d2dmainframe.cxt->CreateSolidColorBrush(D2RGBA(224, 67, 67, 255), &__s_d2dmainframe.br[D2DMainFrame::CLOSE_MOUSE_FLOAT]);
//
//		//st.btn[0] = FRectF(0, 0, 26, 20); // MINI BUTTON
//		//st.btn[1] = FRectF(0, 0, 27, 20); // MAX BUTTON
//		//st.btn[2] = FRectF(0, 0, 45, 20); // CLOSE BUTTON
//
//		__s_d2dmainframe.btnStat = 0;
//
//		//st.title = WINDOW_TITLE;
//
//		//__s_d2dmainframe.factory.Release();
//		//__s_d2dmainframe.wrfactory.Release();
//	}
//
//}

void CTextCell::OnEdit(const EventArgs& e)
{

	CRect rcEdit(m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(GetRect()));
	UINT id[] = { IDB_PNG1 };
	auto spCell = std::static_pointer_cast<CTextCell>(CSheet::Cell(m_pRow, m_pColumn));
	m_pEdit = new V4::D2DWindow(
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
	HWND hwnd = m_pEdit->CreateD2DWindow(0, e.WindowPtr->m_hWnd, WS_CHILD | WS_VISIBLE, rcEdit, id, _countof(id));

	//auto IdleMessage = [](HWND hwnd, UINT msg, UINT_PTR id, DWORD time) {
	//	if (IDLE_TIMER_ID == id)
	//		SendMessage(hwnd, WM_D2D_IDLE, 0, 0);
	//};

	//::SetTimer(e.WindowPtr->m_hWnd, IDLE_TIMER_ID, IDLE_TIME, IdleMessage);


	//CRect rcEdit(m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(InnerBorder2Content(CenterBorder2InnerBorder(GetRect()))));
	//auto spCell = std::static_pointer_cast<CTextCell>(CSheet::Cell(m_pRow, m_pColumn));
	//SetState(UIElementState::Hot);//During Editing, Keep Hot
	//m_pEdit = new CInplaceEdit(
	//	m_pSheet->GetGridPtr(),
	//	[spCell]() -> std::basic_string<TCHAR>{
	//		return spCell->GetString();
	//	},
	//	[spCell](const std::basic_string<TCHAR>& str) -> void{
	//		spCell->SetString(str);
	//	},
	//	[spCell](const std::basic_string<TCHAR>& str) -> void{
	//		if(spCell->CanSetStringOnEditing()){
	//			spCell->SetString(str);
	//		}
	//	},
	//	[spCell]()->void{
	//		spCell->m_pEdit=NULL;
	//		spCell->SetState(UIElementState::Normal);//After Editing, Change Normal
	//	},
	//	m_spProperty->Format->Font.GetGDIFont(),
	//	GetFormat());

	//m_pEdit->Create(m_pSheet->GetGridPtr()->m_hWnd, rcEdit);
	//m_pEdit->SetWindowText(GetString().c_str());
	//rcEdit.MoveToXY(0, 0);
	////	CRect rcRect(m_pEdit->GetRect());
	////	CRect rcPadding(m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(*(m_spProperty->Padding)));
	////	rcEdit.DeflateRect(rcPadding);
	//m_pEdit->SetRect(rcEdit);
	////	m_pEdit->SetMargins(0, 0);// m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(*(m_spProperty->Padding)).left, m_pSheet->GetGridPtr()->GetDirect()->Dips2Pixels(*(m_spProperty->Padding)).right);
	//m_pEdit->SetFocus();
	//m_pEdit->SetSel(0, -1);
	//m_pEdit->ShowWindow(SW_SHOW);
}

void CTextCell::PaintBackground(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	if(m_pEdit){
		CRect rcCell(direct.Dips2Pixels(GetRect()));
		CRect rcEdit; ::GetWindowRect(m_pEdit->hWnd_, &rcEdit);
		rcEdit = m_pSheet->GetGridPtr()->ScreenToClientRect(rcEdit);
		if(rcCell!=rcEdit){
			::MoveWindow(m_pEdit->hWnd_, rcCell.left, rcCell.top, rcCell.Width(), rcCell.Height(), FALSE);
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