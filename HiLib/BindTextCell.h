#pragma once
#include "TextCell.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"

template<typename T> class CBindRow;
template<typename T> class CBindTextColumn;

template<typename T>
class CBindTextCell :public CTextCell
{
public:
	template<typename... Args>
	CBindTextCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, Args... args)
		:CTextCell(pSheet, pRow, pColumn, args...){}

	virtual ~CBindTextCell() = default;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<T>*>(this->m_pRow);
		auto pBindColumn = static_cast<CBindTextColumn<T>*>(this->m_pColumn);
		return pBindColumn->GetGetter()(pBindRow->GetItem<T>());
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<T>*>(this->m_pRow);
		auto pBindColumn = static_cast<CBindTextColumn<T>*>(this->m_pColumn);
		pBindColumn->GetSetter()(pBindRow->GetItem<T>(), str);
	}
};

template<typename T>
class CBindTextCell2 :public CTextCell
{
private:
	std::function<std::wstring()> m_getter;
	std::function<void(const std::wstring&)> m_setter;
public:
	template<typename... Args>
	CBindTextCell2(CGridView* pSheet, CRow* pRow, CColumn* pColumn,  
		std::function<std::wstring()> getter,
		std::function<void(const std::wstring&)> setter,
		Args... args)
		:CTextCell(pSheet, pRow, pColumn, args...), m_getter(getter), m_setter(setter)
	{
	}

	virtual ~CBindTextCell2() = default;

	virtual std::wstring GetString() override
	{
		return m_getter();
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		m_setter(str);
	}
};