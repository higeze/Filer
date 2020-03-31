#pragma once

#include "observable.h"

template<typename TItem>
class IBindSheet
{
public:
	virtual observable_vector<TItem>& GetItemsSource() = 0;
};