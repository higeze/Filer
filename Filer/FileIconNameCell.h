#pragma once
#include "TextCell.h"
#include "Textbox.h"
#include "Direct2DWrite.h"
#include "FileIconCache.h"

class CShellFile;

/*************************/
/* CFileIconNameCellBase */
/*************************/
template<typename... TItems>
class CFileIconNameCellBase :public CTextCell, public std::enable_shared_from_this<CFileIconNameCellBase<TItems...>>
{
protected:
	mutable boost::signals2::connection m_conDelayUpdateAction;
public:
	CFileIconNameCellBase(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit){ }
	virtual ~CFileIconNameCellBase() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile()
	{
		if (auto pBindRow = dynamic_cast<CBindRow<TItems...>*>(m_pRow)) {
			return std::get<std::shared_ptr<CShellFile>>(pBindRow->GetTupleItems());
		} else {
			return nullptr;
		}
	}

	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override
	{
		//Paint Icon
		auto spFile = GetShellFile();
		d2dw::CRectF rcIcon = GetIconSizeF(pDirect);
		rcIcon.MoveToXY(rcPaint.left, rcPaint.top);

		std::weak_ptr<CFileIconNameCellBase> wp(shared_from_this());
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

		pDirect->DrawBitmap(pDirect->GetIconCachePtr()->GetFileIconBitmap(spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), updated), rcIcon);

		//Space
		FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;

		//Paint Text
		d2dw::CRectF rcText(rcIcon.right + space, rcPaint.top, rcPaint.right, rcPaint.bottom);
		CTextCell::PaintContent(pDirect, rcText);
	}

	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect) override
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

	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* pDirect) override
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

	virtual d2dw::CRectF GetEditRect() const override
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

	virtual void OnEdit(const EventArgs& e) override
	{
		m_pSheet->GetGridPtr()->BeginEdit(this);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }

protected:
	d2dw::CSizeF GetIconSizeF(d2dw::CDirect2DWrite* pDirect)const
	{
		return pDirect->Pixels2Dips(GetIconSize(pDirect));
	}

	CSize GetIconSize(d2dw::CDirect2DWrite* pDirect)const
	{
		return CSize(16, 16);
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
};

/*************************/
/* CFileIconDispNameCell */
/*************************/
template<typename... TItems>
class CFileIconDispNameCell:public CFileIconNameCellBase<TItems...>
{
public:
	using CFileIconNameCellBase::CFileIconNameCellBase;
	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetFileNameWithoutExt();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetFileNameWithoutExt(str, m_pSheet->GetGridPtr()->m_hWnd);

	}
};

/*************************/
/* CFileIconPathNameCell */
/*************************/
template<typename... TItems>
class CFileIconPathNameCell :public CFileIconNameCellBase<TItems...>
{
public:
	using CFileIconNameCellBase::CFileIconNameCellBase;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetPathNameWithoutExt();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetFileNameWithoutExt(str, m_pSheet->GetGridPtr()->m_hWnd);
	}
};

/*********************/
/* CFileIconPathCell */
/*********************/
template<typename... TItems>
class CFileIconPathCell :public CFileIconNameCellBase<TItems...>
{
public:
	using CFileIconNameCellBase::CFileIconNameCellBase;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetPath();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		//Do Nothing
	}
};



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