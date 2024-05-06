#pragma once
#include "TextCell.h"
#include "BindRow.h"
#include "ShellFile.h"
#include "RenameInfo.h"
#include "named_arguments.h"

/********************/
/* CFileDispExtCell */
/********************/
template<typename T>
class CFileDispExtCell:public CTextCell
{
public:
	CFileDispExtCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CTextCell(pSheet, pRow, pColumn, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit)
	{
	}
	virtual ~CFileDispExtCell(void){}

	std::wstring GetString()
	{
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetDispExt();
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetExt(str, m_pGrid->GetWndPtr()->m_hWnd);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }
};

/********************/
/* CFilePathExtCell */
/********************/
template<typename T>
class CFilePathExtCell :public CTextCell
{
public:
	CFilePathExtCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CTextCell(pSheet, pRow, pColumn, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit)
	{
	}
	virtual ~CFilePathExtCell() = default;

	std::wstring GetString()
	{
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetPathExt();
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetExt(str, m_pGrid->GetWndPtr()->m_hWnd);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }

};

/********************/
/* CFilePathExtCell */
/********************/
template<typename T>
class CFileReextCell :public CTextCell
{
public:
	CFileReextCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CTextCell(pSheet, pRow, pColumn, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit)
	{
	}
	virtual ~CFileReextCell() = default;

	std::wstring GetString()
	{
		auto pFileRow = static_cast<CBindRow<T>*>(m_pRow);
		return pFileRow->GetItem<RenameInfo>().Ext;
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pFileRow = static_cast<CBindRow<T>*>(m_pRow);
		pFileRow->GetItem<RenameInfo>().Ext = str;
	}
	virtual bool CanSetStringOnEditing()const override { return false; }

};

