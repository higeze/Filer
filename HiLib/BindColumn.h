#pragma once
#include "Column.h"
#include "any_tuple.h"

class CBindColumn :public CColumn
{
public:
	CBindColumn(CSheet* pSheet)
		:CColumn(pSheet){ }

	any_tuple& GetTupleItems();

	const any_tuple& GetTupleItems() const;

	template<typename T> 
	T& GetItem()
	{
		return this->GetTupleItems().get<T>();
	}

	template<typename T> 
	const T& GetItem() const
	{
		return const_cast<T&>(this->GetItem<T>());
	}

	std::shared_ptr<CCell>& Cell(CRow* pRow);

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);

};

