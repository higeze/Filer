#pragma once
#include "TextCell.h"
#include "IImageColumn.h"
#include "CellTextBox.h"
#include "Direct2DWrite.h"

#include "D2DFileIconDrawer.h"
#include "D2DThumbnailDrawer.h"

class CShellFile;
//template <typename... TItems> class CFileNameColumn;

/*************************/
/* CFileIconNameCellBase */
/*************************/
template<typename... TItems>
class CFileNameCellBase :public CTextCell//, public std::enable_shared_from_this<CFileIconNameCellBase<TItems...>>
{
protected:
	mutable sigslot::connection m_conDelayUpdateAction;

public:
	CFileNameCellBase(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit){ }
	virtual ~CFileNameCellBase() = default;
protected:
	std::shared_ptr<CShellFile> GetShellFile()
	{
		if (auto pBindRow = dynamic_cast<CBindRow<TItems...>*>(m_pRow)) {
			return std::get<std::shared_ptr<CShellFile>>(pBindRow->GetTupleItems());
		} else {
			return nullptr;
		}
	}

	UINT32 GetImageSize() const
	{
		if (auto p = dynamic_cast<IImageColumn*>(this->m_pColumn)) {
			return p->GetImageSize();
		} else {
			return 0;
		}
	}

	FLOAT GetImageSizeF() const
	{
		return static_cast<FLOAT>(GetImageSize());
	}

	std::wstring GetViewString()
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
public:
	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override
	{
		//Paint Image
		std::shared_ptr<CShellFile> spFile = this->GetShellFile();
		UINT32 size = this->GetImageSize();

		auto updated = [wp = std::weak_ptr(std::dynamic_pointer_cast<CFileNameCell<TItems...>>(shared_from_this())) ]()->void {
			if (auto sp = wp.lock()) {
				sp->m_conDelayUpdateAction = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
					[wp]()->void {
						if (auto sp = wp.lock()) {
							sp->OnPropertyChanged(L"value");
						}
					});
				sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
			}
		};

		if (size == 16u) {
			pDirect->GetFileIconDrawerPtr()->DrawFileIconBitmap(pDirect, rcPaint.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
		} else {
			pDirect->GetFileThumbnailDrawerPtr()->DrawThumbnailBitmap(pDirect, ThumbnailBmpKey{.Size = size , .Name = spFile->GetPath()}, rcPaint.LeftTop());
		}

		//Space
		FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;

		//Paint Text
		CRectF rcText(rcPaint.left + size + space, rcPaint.top, rcPaint.right, rcPaint.bottom);
		CTextCell::PaintContent(pDirect, rcText);
	}

	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override
	{
		//Calc Icon Size
		CSizeF imageSize(this->GetImageSizeF(), this->GetImageSizeF());
		//Space
		FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
		//Calc Text Size
		CRectF rcCenter(0, 0, m_pColumn->GetWidth(), 0);
		CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));
		CSizeF textSize = pDirect->CalcTextSizeWithFixedWidth(*(m_spCellProperty->Format), GetViewString(), rcContent.Width() - imageSize.width - space);
		//Return
		return CSizeF(imageSize.width + space + textSize.width, (std::max)(imageSize.height, textSize.height));
	}

	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override
	{
		//Calc Icon Size
		CSizeF imageSize(this->GetImageSizeF(), this->GetImageSizeF());
		//Space
		FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
		//Calc Text Size
		CSizeF textSize = pDirect->CalcTextSize(*(m_spCellProperty->Format), GetViewString());
		//Return
		return CSizeF(imageSize.width + space + textSize.width, (std::max)(imageSize.height, textSize.height));
	}

	virtual CRectF GetEditRect() const override
	{
		//Icon Size
		CSizeF imageSize(this->GetImageSizeF(), this->GetImageSizeF());
		//Space
		FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
		//Edit Rect
		CRectF rcEdit(GetRectInWnd());
		rcEdit.left += imageSize.width + space;
		return rcEdit;
	}

	virtual void OnEdit(const Event& e) override
	{
		m_pSheet->GetGridPtr()->BeginEdit(this);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }
};

/*****************/
/* CFileNameCell */
/*****************/
template<typename... TItems>
class CFileNameCell:public CFileNameCellBase<TItems...>
{
public:
	using CFileNameCellBase<TItems...>::CFileNameCellBase;
	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetFileNameWithoutExt();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetFileNameWithoutExt(str, this->m_pSheet->GetWndPtr()->m_hWnd);

	}
};

/*************************/
/* CFileIconPathNameCell */
/*************************/
template<typename... TItems>
class CFileIconPathNameCell :public CFileNameCellBase<TItems...>
{
public:
	using CFileNameCellBase<TItems...>::CFileNameCellBase;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetPathNameWithoutExt();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetFileNameWithoutExt(str, this->m_pSheet->GetWndPtr()->m_hWnd);
	}
};

/*********************/
/* CFileIconPathCell */
/*********************/
template<typename... TItems>
class CFileIconPathCell :public CFileNameCellBase<TItems...>
{
public:
	using CFileNameCellBase<TItems...>::CFileNameCellBase;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetPath();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		//Do Nothing
	}
};

struct RenameInfo;

/*******************/
/* CFileRenameCell */
/*******************/
template<typename... TItems>
class CFileRenameCell :public CTextCell
{
public:
	CFileRenameCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit)
	{
	}

	virtual ~CFileRenameCell(void) {}

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return pBindRow->GetItem<RenameInfo>().Name;
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		pBindRow->GetItem<RenameInfo>().Name = str;
	}
	virtual bool CanSetStringOnEditing()const override { return false; }

};