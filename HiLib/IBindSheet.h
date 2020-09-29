#pragma once

#include "ReactiveProperty.h"

template<typename... TValueItem>
class IBindSheet
{
public:
	virtual ReactiveVectorProperty<std::tuple<TValueItem...>>& GetItemsSource() = 0;
};