#pragma once

#include <string>
#include <any>

class dependency_property
{
	template<typename _PropType, typename _OwnerType>
	static void reg(std::wstring&& name, _PropType&& default_value)
	{
		std::any an = 1.5;
		float fl = an;

	}
};