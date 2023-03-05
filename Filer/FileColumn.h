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

/***********************/
/* CFileNameColumnBase */
/***********************/
template<typename TCell, typename... TItems>
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
template<typename TCell, typename... TItems>
class CFileNameColumnBase : public CFileColumnBase<TCell, TItems...>, public IImageColumn
{
private:
	UINT32 m_size = 16;
public:
	using CFileColumnBase<TCell, TItems...>::CFileColumnBase;
	UINT32 GetImageSize() const override { return m_size; }
	void SetImageSize(const UINT32& size) override { m_size = size; }
};

/*******************/
/* CFileNameColumn */
/*******************/
template<typename... TItems>
using CFileNameColumn = CFileNameColumnBase<CFileNameCell<TItems...>, TItems...>;

/***********************/
/* CFilePathNameColumn */
/***********************/
template<typename... TItems>
using CFilePathNameColumn = CFileNameColumnBase<CFileIconPathNameCell<TItems...>, TItems...>;

/*************************/
/* CFilePathRenameColumn */
/*************************/
template<typename... TItems>
using CFilePathRenameColumn = CFileColumnBase<CFileRenameCell<TItems...>, TItems...>;

/***********************/
/* CFileIconPathColumn */
/***********************/
template<typename... TItems>
using CFileIconPathColumn = CFileNameColumnBase<CFileIconPathCell<TItems...>, TItems...>;

/**********************/
/* CFileDispExtColumn */
/**********************/
template<typename... TItems>
using CFileDispExtColumn = CFileColumnBase<CFileDispExtCell<TItems...>, TItems...>;

/**********************/
/* CFilePathExtColumn */
/**********************/
template<typename... TItems>
using CFilePathExtColumn = CFileColumnBase<CFilePathExtCell<TItems...>, TItems...>;

/********************/
/* CFileReextColumn */
/********************/
template<typename... TItems>
using CFileReextColumn = CFileColumnBase<CFileReextCell<TItems...>, TItems...>;


