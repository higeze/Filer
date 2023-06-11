#pragma once
#include "reactive_property.h"
#include "reactive_binding.h"

template<class T>
rxcpp::composite_subscription reactive_binding(reactive_property<T>& left, reactive_property<T>& right)
{
    rxcpp::composite_subscription ret;
    {
        ret.add(
        left.subscribe([&](const T& value) {
            right.set(value);
        }));
    }
    {
        ret.add(
        right.subscribe([&](const T& value) {
            left.set(value);
        }));
    }
    return ret;
}

template<class T>
rxcpp::composite_subscription reactive_command_binding(reactive_command<T>& dest, reactive_command<T>& src)
{
	return dest.subscribe(
		[&](T value)->void
		{
			src.execute(value);
		});
}