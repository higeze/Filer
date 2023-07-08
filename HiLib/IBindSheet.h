#pragma once

#include "ReactiveProperty.h"
#include "reactive_vector.h"

template<typename... TValueItem>
class IBindSheet
{
public:
	virtual ReactiveVectorProperty<std::tuple<TValueItem...>>& GetItemsSource() = 0;
};

template<typename... TValueItem>
class IBindSheet2
{
public:
	virtual reactive_vector_ptr<std::tuple<TValueItem...>>& GetItemsSource() = 0;
};