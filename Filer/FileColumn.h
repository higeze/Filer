#pragma once
#include "MapColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileIconNameCell.h"
#include "PathCell.h"
#include "CellProperty.h"

#include "FileExtCell.h"

/***********************/
/* CFileNameColumnBase */
/***********************/
template<typename TCell, typename... TItems>
class CFileTemplateColumn : public CMapColumn
{
private:
	std::wstring m_header;
public:
	template<typename... Args>
	CFileTemplateColumn(CSheet* pSheet, const std::wstring& header, Args... args)
		:CMapColumn(pSheet, args...), m_header(header)
	{
		m_minLength = ::get(arg<"minwidth"_s>(), args..., default_(30.f));
	}
	virtual ~CFileTemplateColumn() = default;

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CPathCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = m_header);
	}

	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<TCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

};

/***********************/
/* CFileDispNameColumn */
/***********************/

template<typename... TItems>
using CFileDispNameColumn = CFileTemplateColumn<CFileIconDispNameCell<TItems...>, TItems...>;
//class CFileDispNameColumn: public CFileTemplateColumn<TItems...>
//{
//public:
//	template<typename... Args>
//	CFileDispNameColumn(CSheet* pSheet = nullptr, Args... args)
//		:CFileTemplateColumn(pSheet, args...){}
//	virtual ~CFileDispNameColumn() = default;
//
//	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
//	{
//		return std::make_shared<CFileIconDispNameCell<TItems...>>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
//	}
//};

/***********************/
/* CFilePathNameColumn */
/***********************/
template<typename... TItems>
using CFilePathNameColumn = CFileTemplateColumn<CFileIconPathNameCell<TItems...>, TItems...>;
//class CFilePathNameColumn : public CFileTemplateColumn<TItems...>
//{
//public:
//	template<typename... Args>
//	CFilePathNameColumn(CSheet* pSheet = nullptr, Args... args)
//		:CFileTemplateColumn(pSheet, args...){}
//	virtual ~CFilePathNameColumn() = default;
//
//	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
//	{
//		return std::make_shared<CFileIconPathNameCell<TItems...>>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
//	}
//};

/*************************/
/* CFilePathRenameColumn */
/*************************/
template<typename... TItems>
using CFilePathRenameColumn = CFileTemplateColumn<CFileRenameCell<TItems...>, TItems...>;

//class CFilePathRenameColumn : public CFileTemplateColumn
//{
//public:
//	template<typename... Args>
//	CFilePathRenameColumn(CSheet* pSheet = nullptr, Args... args)
//		:CFileTemplateColumn(pSheet, args...){}
//	virtual ~CFilePathRenameColumn(void) {};
//
//	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
//	{
//		return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"Rename");
//	}
//
//	virtual std::shared_ptr<CCell> CFilePathRenameColumn::CellTemplate(CRow* pRow, CColumn* pColumn) override
//	{
//		return std::make_shared<CFileRenameCell<TItems...>>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
//	}
//};

/*************************/
/* CFileIconPathColumn */
/*************************/

template<typename... TItems>
using CFileIconPathColumn = CFileTemplateColumn<CFileIconPathCell<TItems...>, TItems...>;
//{
//public:
//	CFileIconPathColumn(CSheet* pSheet = nullptr);
//	virtual ~CFileIconPathColumn(void) {};
//
//	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
//	{
//		return std::make_shared<CFileIconPathCell<TItems...>>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
//	}
//};

template<typename... TItems>
using CFileDispExtColumn = CFileTemplateColumn<CFileDispExtCell<TItems...>, TItems...>;

template<typename... TItems>
using CFilePathExtColumn = CFileTemplateColumn<CFilePathExtCell<TItems...>, TItems...>;

template<typename... TItems>
using CFileReextColumn = CFileTemplateColumn<CFileReextCell<TItems...>, TItems...>;


