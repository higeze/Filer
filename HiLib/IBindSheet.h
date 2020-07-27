#pragma once

#include "observable.h"

template<typename... TValueItem>
class IBindSheet
{
public:
	virtual observable_vector<std::tuple<TValueItem...>>& GetItemsSource() = 0;
};