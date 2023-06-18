#pragma once
#include "reactive_property.h"
#include "reactive_binding.h"

template<class T>
rxcpp::composite_subscription reactive_binding(reactive_property<T>& src, reactive_property<T>& dst)
{
    dst.set(src.get());
    rxcpp::composite_subscription ret;
    {
        ret.add(
        dst.subscribe([&src](const T& value) {
            src.set(value);
        }));
    }
    {
        ret.add(
        src.subscribe([&dst](const T& value) {
            dst.set(value);
        }));
    }
    return ret;
}

template<class T>
rxcpp::composite_subscription reactive_command_binding(reactive_command<T> src, reactive_command<T> dst)
{
	return dst.subscribe(
		[src](T value)->void
		{
			src.execute(value);
		});
}