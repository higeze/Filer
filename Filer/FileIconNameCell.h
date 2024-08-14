#pragma once
#include "TextCell.h"
#include "IImageColumn.h"
#include "CellTextBox.h"
#include "Direct2DWrite.h"

#include "D2DFileIconDrawer.h"
#include "D2DThumbnailDrawer.h"

class CShellFile;

/*************************/
/* CFileIconNameCellBase */
/*************************/
template<typename T>
class CFileNameCellBase :public CTextCell
{
protected:
	mutable sigslot::connection m_conDelayUpdateAction;

public:
	CFileNameCellBase(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CTextCell(pSheet, pRow, pColumn, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit){ }
	virtual ~CFileNameCellBase() = default;
protected:
	std::shared_ptr<CShellFile> GetShellFile()
	{
		if (auto p = dynamic_cast<CBindRow<T>*>(m_pRow)) {
			return p->GetItem<std::shared_ptr<CShellFile>>();
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
		if (m_pGrid->GetEditPtr() && m_pGrid->GetEditPtr()->GetCellPtr() == this) {
			text = *m_pGrid->GetEditPtr()->Text;
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

		auto updated = [wp = std::weak_ptr(std::dynamic_pointer_cast<CFileNameCellBase<T>>(shared_from_this())) ]()->void {
			if (auto sp = wp.lock()) {
				sp->m_conDelayUpdateAction = sp->GetGridPtr()->SignalPreDelayUpdate.connect(
					[wp]()->void {
						if (auto sp = wp.lock()) {
							sp->OnPropertyChanged(L"value");
						}
					});
				sp->GetGridPtr()->DelayUpdate();
			}
		};

		if (size == 16u) {
			pDirect->GetFileIconDrawerPtr()->DrawFileIconBitmap(pDirect, rcPaint.LeftTop(), spFile.get(), updated);
		} else {
			pDirect->GetFileThumbnailDrawerPtr()->DrawThumbnailBitmap(pDirect, ThumbnailBmpKey{.Size = size , .Name = spFile->GetPath()}, rcPaint.LeftTop());
		}

		//Space
		FLOAT space = GetPadding().left + GetPadding().right;

		//Paint Text
		CRectF rcText(rcPaint.left + size + space, rcPaint.top, rcPaint.right, rcPaint.bottom);
		CTextCell::PaintContent(pDirect, rcText);
	}

	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override
	{
		//Calc Icon Size
		CSizeF imageSize(this->GetImageSizeF(), this->GetImageSizeF());
		//Space
		FLOAT space = GetPadding().left + GetPadding().right;
		//Calc Text Size
		CRectF rcCenter(0, 0, m_pColumn->GetWidth(), 0);
		CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));
		CSizeF textSize = pDirect->CalcTextSizeWithFixedWidth(GetFormat(), GetViewString(), rcContent.Width() - imageSize.width - space);
		//Return
		return CSizeF(imageSize.width + space + textSize.width, (std::max)(imageSize.height, textSize.height));
	}

	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override
	{
		//Calc Icon Size
		CSizeF imageSize(this->GetImageSizeF(), this->GetImageSizeF());
		//Space
		FLOAT space = GetPadding().left + GetPadding().right;
		//Calc Text Size
		CSizeF textSize = pDirect->CalcTextSize(GetFormat(), GetViewString());
		//Return
		return CSizeF(imageSize.width + space + textSize.width, (std::max)(imageSize.height, textSize.height));
	}

	virtual CRectF GetEditRect() const override
	{
		//Icon Size
		CSizeF imageSize(this->GetImageSizeF(), this->GetImageSizeF());
		//Space
		FLOAT space = GetPadding().left + GetPadding().right;
		//Edit Rect
		CRectF rcEdit(GetRectInWnd());
		rcEdit.left += imageSize.width + space;
		return rcEdit;
	}

	virtual void OnEdit(const Event& e) override
	{
		m_pGrid->BeginEdit(this);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }
};

/*****************/
/* CFileNameCell */
/*****************/
template<typename T>
class CFileNameCell:public CFileNameCellBase<T>
{
	using bind_row = CBindRow<T>;
public:
	using CFileNameCellBase<T>::CFileNameCellBase;
	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<bind_row*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetDispNameWithoutExt();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<bind_row*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetFileNameWithoutExt(str, this->m_pGrid->GetWndPtr()->m_hWnd);

	}
};

/*************************/
/* CFileIconPathNameCell */
/*************************/
template<typename T>
class CFileIconPathNameCell :public CFileNameCellBase<T>
{
	using bind_row = CBindRow<T>;
public:
	using CFileNameCellBase<T>::CFileNameCellBase;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<bind_row*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetPathNameWithoutExt();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<bind_row*>(this->m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetFileNameWithoutExt(str, this->m_pGrid->GetWndPtr()->m_hWnd);
	}
};

/*********************/
/* CFileIconPathCell */
/*********************/
template<typename T>
class CFileIconPathCell :public CFileNameCellBase<T>
{
	using bind_row = CBindRow<T>;
public:
	using CFileNameCellBase<T>::CFileNameCellBase;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<bind_row*>(this->m_pRow);
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
template<typename T>
class CFileRenameCell :public CTextCell
{
	using bind_row = CBindRow<T>;
public:
	CFileRenameCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CTextCell(pSheet, pRow, pColumn, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit){}

	virtual ~CFileRenameCell(void) {}

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<bind_row*>(this->m_pRow);
		return pBindRow->GetItem<RenameInfo>().Name;
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<bind_row*>(this->m_pRow);
		pBindRow->GetItem<RenameInfo>().Name = str;
	}
	virtual bool CanSetStringOnEditing()const override { return false; }

};