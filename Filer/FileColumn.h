#pragma once
#include "MapColumn.h"
#include "HeaderSortCell.h"
#include "FilterCell.h"
#include "GridView.h"
#include "FileIconNameCell.h"
#include "PathCell.h"
#include "CellProperty.h"

#include "FileExtCell.h"
#include "IImageColumn.h"

/***********************/
/* CFileNameColumnBase */
/***********************/
template<typename TCell, typename T>
class CFileColumnBase : public CMapColumn
{
private:
	std::wstring m_header;
public:
	template<typename... Args>
	CFileColumnBase(CGridView* pSheet = nullptr, const std::wstring& header = L"", Args... args)
		:CMapColumn(pSheet, args...), m_header(header)
	{
		m_minLength = ::get(arg<"minwidth"_s>(), args..., default_(30.f));
	}
	virtual ~CFileColumnBase() = default;

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CPathCell>(m_pGrid, pRow, pColumn);
	}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CHeaderSortCell>(m_pGrid, pRow, pColumn, arg<"text"_s>() = m_header);
	}

	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		auto cell = std::make_shared<CFilterCell>(m_pGrid, pRow, pColumn);
		return cell;
	}

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		auto cell = std::make_shared<TCell>(m_pGrid, pRow, pColumn);
		return cell;
	}
};

/***********************/
/* CFileNameColumnBase */
/***********************/
template<typename TCell, typename T>
class CFileNameColumnBase : public CFileColumnBase<TCell, T>, public IImageColumn
{
private:
	UINT32 m_size = 16;
public:
	using CFileColumnBase<TCell, T>::CFileColumnBase;
	UINT32 GetImageSize() const override { return m_size; }
	void SetImageSize(const UINT32& size) override { m_size = size; }
};

/*******************/
/* CFileNameColumn */
/*******************/
template<typename T>
using CFileNameColumn = CFileNameColumnBase<CFileNameCell<T>, T>;

/***********************/
/* CFilePathNameColumn */
/***********************/
template<typename T>
using CFilePathNameColumn = CFileNameColumnBase<CFileIconPathNameCell<T>, T>;

/*************************/
/* CFilePathRenameColumn */
/*************************/
template<typename T>
using CFilePathRenameColumn = CFileColumnBase<CFileRenameCell<T>, T>;

/***********************/
/* CFileIconPathColumn */
/***********************/
template<typename T>
using CFileIconPathColumn = CFileNameColumnBase<CFileIconPathCell<T>, T>;

/**********************/
/* CFileDispExtColumn */
/**********************/
template<typename T>
using CFileDispExtColumn = CFileColumnBase<CFileDispExtCell<T>, T>;

/**********************/
/* CFilePathExtColumn */
/**********************/
template<typename T>
using CFilePathExtColumn = CFileColumnBase<CFilePathExtCell<T>, T>;

/********************/
/* CFileReextColumn */
/********************/
template<typename T>
using CFileReextColumn = CFileColumnBase<CFileReextCell<T>, T>;


