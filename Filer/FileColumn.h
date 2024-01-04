#pragma once
#include "MapColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileIconNameCell.h"
#include "PathCell.h"
#include "CellProperty.h"

#include "FileExtCell.h"
#include "IImageColumn.h"
#include "FileExtCell.h"

/***********************/
/* CFileNameColumnBase */
/***********************/
template<typename TCell>
class CFileColumnBase : public CMapColumn
{
private:
	std::wstring m_header;
public:
	template<typename... Args>
	CFileColumnBase(CSheet* pSheet = nullptr, const std::wstring& header = L"", Args... args)
		:CMapColumn(pSheet, args...), m_header(header)
	{
		m_minLength = ::get(arg<"minwidth"_s>(), args..., default_(30.f));
	}
	virtual ~CFileColumnBase() = default;

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
		auto cell = std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
		return cell;
	}

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		auto cell = std::make_shared<TCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
		return cell;
	}

public:
	template <class Archive>
	void save(Archive& ar)
	{
		CColumn::save(ar);

		ar("header", m_header);
	}
	template <class Archive>
	void load(Archive& ar)
	{
		CColumn::load(ar);

		ar("header", m_header);
	}

};

/***********************/
/* CFileNameColumnBase */
/***********************/
template<typename TCell>
class CFileNameColumnBase : public CFileColumnBase<TCell>, public IImageColumn
{
private:
	UINT32 m_size = 16;
public:
	using CFileColumnBase<TCell>::CFileColumnBase;
	UINT32 GetImageSize() const override { return m_size; }
	void SetImageSize(const UINT32& size) override { m_size = size; }
};

/*******************/
/* CFileNameColumn */
/*******************/
using CFileNameColumn = CFileNameColumnBase<CFileNameCell>;

/***********************/
/* CFilePathNameColumn */
/***********************/
using CFilePathNameColumn = CFileNameColumnBase<CFileIconPathNameCell>;

/*************************/
/* CFilePathRenameColumn */
/*************************/
using CFilePathRenameColumn = CFileColumnBase<CFileRenameCell>;

/***********************/
/* CFileIconPathColumn */
/***********************/
using CFileIconPathColumn = CFileNameColumnBase<CFileIconPathCell>;

/**********************/
/* CFileDispExtColumn */
/**********************/
using CFileDispExtColumn = CFileColumnBase<CFileDispExtCell>;

/**********************/
/* CFilePathExtColumn */
/**********************/
using CFilePathExtColumn = CFileColumnBase<CFilePathExtCell>;

/********************/
/* CFileReextColumn */
/********************/
using CFileReextColumn = CFileColumnBase<CFileReextCell>;


