#pragma once
#include "TextCell.h"
#include "BindRow.h"
#include "ShellFile.h"
#include "RenameInfo.h"
#include "named_arguments.h"

/********************/
/* CFileDispExtCell */
/********************/
template<typename... TItems>
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
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return std::get<std::shared_ptr<CShellFile>>(pBindRow->GetTupleItems())->GetDispExt();
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return std::get<std::shared_ptr<CShellFile>>(pBindRow->GetTupleItems())->SetExt(str, m_pSheet->GetGridPtr()->m_hWnd);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }
};

/********************/
/* CFilePathExtCell */
/********************/
template<typename... TItems>
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
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return std::get<std::shared_ptr<CShellFile>>(pBindRow->GetTupleItems())->GetPathExt();
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return std::get<std::shared_ptr<CShellFile>>(pBindRow->GetTupleItems())->SetExt(str, m_pSheet->GetGridPtr()->m_hWnd);
	}

	virtual bool CanSetStringOnEditing()const override { return false; }

};

/********************/
/* CFilePathExtCell */
/********************/
template<typename... TItems>
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
		auto pFileRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		return std::get<RenameInfo>(pFileRow->GetTupleItems()).Ext;
	}

	void SetStringCore(const std::wstring& str)
	{
		auto pFileRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		std::get<RenameInfo>(pFileRow->GetTupleItems()).Ext = str;
	}
	virtual bool CanSetStringOnEditing()const override { return false; }

};

