#pragma once
#include "TextCell.h"
#include "BindRow.h"
#include "ShellFile.h"
#include "RenameInfo.h"
#include "named_arguments.h"

/********************/
/* CFileDispExtCell */
/********************/
class CFileDispExtCell:public CTextCell
{
public:
	CFileDispExtCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit)
	{
	}
	virtual ~CFileDispExtCell(void){}

	std::wstring GetString()
	{
		auto pBindRow = static_cast<CBindRow*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetDispExt();
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pBindRow = static_cast<CBindRow*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetExt(str, m_pSheet->GetWndPtr()->m_hWnd);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }
};

/********************/
/* CFilePathExtCell */
/********************/
class CFilePathExtCell :public CTextCell
{
public:
	CFilePathExtCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit)
	{
	}
	virtual ~CFilePathExtCell() = default;

	std::wstring GetString()
	{
		auto pBindRow = static_cast<CBindRow*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->GetPathExt();
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pBindRow = static_cast<CBindRow*>(m_pRow);
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>()->SetExt(str, m_pSheet->GetWndPtr()->m_hWnd);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }

};

/********************/
/* CFilePathExtCell */
/********************/
class CFileReextCell :public CTextCell
{
public:
	CFileReextCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit)
	{
	}
	virtual ~CFileReextCell() = default;

	std::wstring GetString()
	{
		auto pFileRow = static_cast<CBindRow*>(m_pRow);
		return pFileRow->GetItem<RenameInfo>().Ext;
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pFileRow = static_cast<CBindRow*>(m_pRow);
		pFileRow->GetItem<RenameInfo>().Ext = str;
	}
	virtual bool CanSetStringOnEditing()const override { return false; }

};

