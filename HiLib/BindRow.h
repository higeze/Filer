#pragma once
#include "Row.h"
#include "any_tuple.h"

class CBindRow :public CRow
{
public:
	CBindRow(CSheet* pSheet, std::shared_ptr<CellProperty> spProperty)
		:CRow(pSheet, spProperty){ }

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
};