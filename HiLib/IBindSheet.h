#pragma once
#include "reactive_vector.h"

//template<typename... TValueItem>
//class IBindSheet
//{
//public:
//	virtual ReactiveVectorProperty<std::tuple<TValueItem...>>& GetItemsSource() = 0;
//};

template<typename T>
class IBindSheet
{
public:
	virtual reactive_vector_ptr<T>& GetItemsSource() = 0;
};