#include "FileIconNameCell.h"
#include "ShellFile.h"
#include "FileRow.h"
#include "GridView.h"
#include "CellProperty.h"
#include "FileIconCache.h"
#include "Textbox.h"

CFileIconNameCell::CFileIconNameCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CParameterCell(pSheet, pRow, pColumn, spProperty){}

std::wstring CFileIconNameCell::GetString()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetFileNameWithoutExt();
}

void CFileIconNameCell::SetStringCore(const std::basic_string<TCHAR>& str)
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	pFileRow->GetFilePointer()->SetFileNameWithoutExt(str);

}

d2dw::CSizeF CFileIconNameCell::GetIconSizeF(d2dw::CDirect2DWrite* pDirect)const
{
	return pDirect->Pixels2Dips(GetIconSize(pDirect));
}

CSize CFileIconNameCell::GetIconSize(d2dw::CDirect2DWrite* pDirect)const
{
	return CSize(16, 16);
}

std::shared_ptr<CShellFile> CFileIconNameCell::GetShellFile()
{
	if (auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)) {
		return pFileRow->GetFilePointer();
	} else {
		return nullptr;
	}
}

void CFileIconNameCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	//Paint Icon
	auto spFile = GetShellFile();
	d2dw::CRectF rcIcon = GetIconSizeF(pDirect);
	rcIcon.MoveToXY(rcPaint.left, rcPaint.top);

	std::weak_ptr<CFileIconNameCell> wp(shared_from_this());
	std::function<void()> updated = [wp]()->void {
		if (auto sp = wp.lock()) {
			auto con = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
				[wp]()->void {
				if (auto sp = wp.lock()) {
					sp->OnPropertyChanged(L"value");
				}
			});
			sp->m_conDelayUpdateAction = con;
			sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
		}
	};

	pDirect->DrawBitmap(direct.GetIconCachePtr()->GetFileIconBitmap(spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetExt(), updated), rcIcon);

	//Space
	FLOAT space = m_spProperty->Padding->left + m_spProperty->Padding->right;

	//Paint Text
	d2dw::CRectF rcText(rcIcon.right + space, rcPaint.top, rcPaint.right, rcPaint.bottom);

	CTextCell::PaintContent(pDirect, rcText);
}

d2dw::CSizeF CFileIconNameCell::MeasureContentSize(d2dw::CDirect2DWrite& direct)
{
	//Calc Icon Size
	d2dw::CSizeF iconSize(GetIconSizeF(direct));
	//Space
	FLOAT space = m_spProperty->Padding->left + m_spProperty->Padding->right;
	//Calc Text Size
	d2dw::CSizeF textSize = CTextCell::MeasureContentSize(direct);
	//Return
	return d2dw::CSizeF(iconSize.width + space + textSize.width, (std::max)(iconSize.height, textSize.height));
}

d2dw::CSizeF CFileIconNameCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	//Calc Icon Size
	d2dw::CSizeF iconSize(GetIconSizeF(direct));
	//Space
	FLOAT space = m_spProperty->Padding->left + m_spProperty->Padding->right;
	//Calc Text Size
	d2dw::CRectF rcCenter(0, 0, m_pColumn->GetWidth(), 0);
	d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));
	std::wstring text = GetString();
	if (text.empty()) { text = L"a"; }
	d2dw::CSizeF textSize = direct.CalcTextSizeWithFixedWidth(*(m_spProperty->Format), text, rcContent.Width() - iconSize.width - space);
	//Return
	return d2dw::CSizeF(iconSize.width + space + textSize.width, (std::max)(iconSize.height, textSize.height));
}

void CFileIconNameCell::OnEdit(const EventArgs& e)
{
	//Icon Size
	CSize iconSize(GetIconSize(*(m_pSheet->GetGridPtr()->GetDirectPtr())));
	//Space
	int space = m_pSheet->GetGridPtr()->GetDirectPtr()->Dips2PixelsX(m_spProperty->Padding->left + m_spProperty->Padding->right);
	//Edit Rect
	CRect rcEdit(m_pSheet->GetGridPtr()->GetDirectPtr()->Dips2Pixels(m_pSheet->GetGridPtr()->GetDirectPtr()->LayoutRound(GetRect())));
	rcEdit.left += iconSize.cx + space;

	SetState(UIElementState::Hot);//During Editing, Keep Hot
	//CRect rcEdit(m_pSheet->GetGridPtr()->GetDirectPtr()->Dips2Pixels(GetRect()));
	auto spCell = std::static_pointer_cast<CTextCell>(CSheet::Cell(m_pRow, m_pColumn));

	//m_pEdit = new CTextboxWnd(
	//	m_spProperty,
	//	[spCell]() -> std::basic_string<TCHAR> {
	//		return spCell->GetString();
	//	},
	//	[spCell](const std::basic_string<TCHAR>& str) -> void {
	//		spCell->SetString(str);
	//	},
	//		[spCell](const std::basic_string<TCHAR>& str) -> void {
	//		if (spCell->CanSetStringOnEditing()) {
	//			spCell->SetString(str);
	//		}
	//	},
	//		[spCell]()->void {
	//		spCell->SetEditPtr(nullptr);
	//		spCell->SetState(UIElementState::Normal);//After Editing, Change Normal
	//	}
	//);
	//m_pEdit->Create(e.WindowPtr->m_hWnd, rcEdit);
}

void CFileIconNameCell::PaintBackground(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	if (m_pEdit) {
		//Icon Size
		CSize iconSize(GetIconSize(pDirect));
		//Space
		int space = m_pSheet->GetGridPtr()->GetDirectPtr()->Dips2PixelsX(m_spProperty->Padding->left + m_spProperty->Padding->right);
		//Inner Rect
		d2dw::CRectF rcText(InnerBorder2Content(CenterBorder2InnerBorder(GetRect())));
		rcText.left += iconSize.cx + space;

		//d2dw::CRectF rcEdit(direct.Pixels2Dips(m_pSheet->GetGridPtr()->ScreenToClientRect(m_pEdit->GetWindowRect())));

		//if (rcText != rcEdit) {
		//	m_pEdit->MoveWindow(direct.Dips2Pixels(rcText), FALSE);
		//}
	}
	CCell::PaintBackground(pDirect, rcPaint);
}

