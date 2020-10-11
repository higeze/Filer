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
class CFileIconNameCellBase :public CTextCell//, public std::enable_shared_from_this<CFileIconNameCellBase<TItems...>>
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

	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override
	{
		//Paint Icon
		auto spFile = GetShellFile();
		CRectF rcIcon = GetIconSizeF(pDirect);
		rcIcon.MoveToXY(rcPaint.left, rcPaint.top);

		//TODOHIGH Not thread safe
		std::weak_ptr<CFileIconNameCellBase> wp(std::dynamic_pointer_cast<CFileIconNameCellBase>(shared_from_this()));
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
		CRectF rcText(rcIcon.right + space, rcPaint.top, rcPaint.right, rcPaint.bottom);
		CTextCell::PaintContent(pDirect, rcText);
	}

	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override
	{
		//Calc Icon Size
		CSizeF iconSize(GetIconSizeF(pDirect));
		//Space
		FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
		//Calc Text Size
		CRectF rcCenter(0, 0, m_pColumn->GetWidth(), 0);
		CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));

		CSizeF textSize = pDirect->CalcTextSizeWithFixedWidth(*(m_spCellProperty->Format), GetViewString(), rcContent.Width() - iconSize.width - space);
		//Return
		return CSizeF(iconSize.width + space + textSize.width, (std::max)(iconSize.height, textSize.height));
	}

	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override
	{
		//Calc Icon Size
		CSizeF iconSize(GetIconSizeF(pDirect));
		//Space
		FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
		//Calc Text Size
		CSizeF textSize = pDirect->CalcTextSize(*(m_spCellProperty->Format), GetViewString());
		//Return
		return CSizeF(iconSize.width + space + textSize.width, (std::max)(iconSize.height, textSize.height));
	}

	virtual CRectF GetEditRect() const override
	{
		//Icon Size
		CSizeF iconSize(GetIconSizeF(m_pSheet->GetWndPtr()->GetDirectPtr()));
		//Space
		FLOAT space = m_spCellProperty->Padding->left + m_spCellProperty->Padding->right;
		//Edit Rect
		CRectF rcEdit(GetRectInWnd());
		rcEdit.left += iconSize.width + space;
		return rcEdit;
	}

	virtual void OnEdit(const Event& e) override
	{
		m_pSheet->GetGridPtr()->BeginEdit(this);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }

protected:
	CSizeF GetIconSizeF(CDirect2DWrite* pDirect)const
	{
		return pDirect->Pixels2Dips(GetIconSize(pDirect));
	}

	CSize GetIconSize(CDirect2DWrite* pDirect)const
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
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetFileNameWithoutExt(str, m_pSheet->GetWndPtr()->m_hWnd);

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
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetFileNameWithoutExt(str, m_pSheet->GetWndPtr()->m_hWnd);
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