#include "FilterCell.h"
#include "InplaceEdit.h"
#include "Sheet.h"
#include "CellProperty.h"
#include "Column.h"
#include "GridView.h"

CFilterCell::CFilterCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, CMenu* pMenu)
	:CEditableCell(pSheet, pRow, pColumn, spProperty,pMenu){ }

CFilterCell::~CFilterCell()
{
	boost::asio::deadline_timer* pTimer = static_cast<CGridView*>(m_pSheet)->GetFilterTimerPtr();
	pTimer->cancel();
}

std::wstring CFilterCell::GetString()
{
	return m_pColumn->GetFilter();
}

void CFilterCell::SetString(const std::wstring& str)
{
	//Filter cell undo redo is set when Post WM_FILTER
	if(GetString()!=str){
		std::wstring newString = str;
		boost::asio::deadline_timer* pTimer = static_cast<CGridView*>(m_pSheet)->GetFilterTimerPtr();
		pTimer->expires_from_now(boost::posix_time::milliseconds(500));
		CCell* pCell = this;
		HWND hWnd = m_pSheet->GetGridPtr()->m_hWnd;
		pTimer->async_wait([hWnd,pCell,newString](const boost::system::error_code& error)->void{

			if(error == boost::asio::error::operation_aborted){
				::OutputDebugStringA("timer canceled\r\n");
			}else{
				::OutputDebugStringA("timer filter\r\n");
				pCell->CCell::SetString(newString);	
				::PostMessage(hWnd,WM_FILTER,NULL,NULL);
			}
		});
	}
}

void CFilterCell::SetStringCore(const std::wstring& str)
{
	m_pColumn->SetFilter(str);
}

void CFilterCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	std::wstring str=GetString();
	if(!str.empty()){
		direct.DrawTextLayout(*(m_spProperty->Format), str, rcPaint);
	}else{
		str = L"Filter items...";
		d2dw::FormatF filterFnC(
			m_spProperty->Format->Font.FamilyName, m_spProperty->Format->Font.Size,
			210.0f/255,210.0f/255,210.0f/255, 1.0f);
		direct.DrawTextLayout(filterFnC, str, rcPaint);
	}
}