#include "FileIconNameCell.h"
#include "ShellFile.h"
#include "FileRow.h"
#include "GridView.h"
#include "CellProperty.h"
#include "FileIconCache.h"
#include "Textbox.h"

std::wstring CFileIconNameCell::GetString()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetFileNameWithoutExt();
}

void CFileIconNameCell::SetStringCore(const std::wstring& str)
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	pFileRow->GetFilePointer()->SetFileNameWithoutExt(str, m_pSheet->GetGridPtr()->m_hWnd);

}

d2dw::CSizeF CFileIconNameCell::GetIconSizeF(d2dw::CDirect2DWrite* pDirect)const
{
	return pDirect->Pixels2Dips(GetIconSize(pDirect));
}

CSize CFileIconNameCell::GetIconSize(d2dw::CDirect2DWrite* pDirect)const
{
	return CSize(16, 16);
}

std::wstring CFileIconNameCell::GetViewString()
{
	std::wstring text;
	if (m_pSheet->GetGridPtr()->GetEditPtr() && m_pSheet->GetGridPtr()->GetEditPtr()->GetCellPtr() == this) {
		text = m_pSheet->GetGridPtr()->GetEditPtr()->GetText();
	} else {
		text = GetString();
	}
	if (text.empty()) { text = L"a"; }

	return text;
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
	rcIcon.MoveToXY(rcPaint.left, rcPaint.top);//TODOTODO

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

	pDirect->DrawBitmap(pDirect->GetIconCachePtr()->GetFileIconBitmap(spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetExt(), updated), rcIcon);

	//Space
	FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;

	//Paint Text
	d2dw::CRectF rcText(rcIcon.right + space, rcPaint.top, rcPaint.right, rcPaint.bottom);
	CTextCell::PaintContent(pDirect, rcText);
}

d2dw::CSizeF CFileIconNameCell::MeasureContentSize(d2dw::CDirect2DWrite* pDirect)
{
	//Calc Icon Size
	d2dw::CSizeF iconSize(GetIconSizeF(pDirect));
	//Space
	FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
	//Calc Text Size
	d2dw::CSizeF textSize = pDirect->CalcTextSize(*(m_spCellProperty->Format), GetViewString());
	//Return
	return d2dw::CSizeF(iconSize.width + space + textSize.width, (std::max)(iconSize.height, textSize.height));
}

d2dw::CSizeF CFileIconNameCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	//Calc Icon Size
	d2dw::CSizeF iconSize(GetIconSizeF(pDirect));
	//Space
	FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
	//Calc Text Size
	d2dw::CRectF rcCenter(0, 0, m_pColumn->GetWidth(), 0);
	d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));

	d2dw::CSizeF textSize = pDirect->CalcTextSizeWithFixedWidth(*(m_spCellProperty->Format), GetViewString(), rcContent.Width() - iconSize.width - space);
	//Return
	return d2dw::CSizeF(iconSize.width + space + textSize.width, (std::max)(iconSize.height, textSize.height));
}

d2dw::CRectF CFileIconNameCell::GetEditRect() const
{
	//Icon Size
	d2dw::CSizeF iconSize(GetIconSizeF(m_pSheet->GetGridPtr()->GetDirectPtr()));
	//Space
	FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
	//Edit Rect
	d2dw::CRectF rcEdit(GetRect());
	rcEdit.left += iconSize.width + space;
	return rcEdit;
}

void CFileIconNameCell::OnEdit(const EventArgs& e)
{
	m_pSheet->GetGridPtr()->BeginEdit(this);
}


