#pragma once
#include "reactive_vector.h"
#include "JsonSerializer.h"

struct ExeExtension
{
	auto operator<=>(const ExeExtension&) const = default;

	std::wstring Name;
	std::wstring Path;
	std::wstring Parameter;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(
		ExeExtension,
		Name,
		Path,
		Parameter)
};

struct ExeExtensionProperty
{
public:
	reactive_vector_ptr<std::tuple<ExeExtension>> ExeExtensions;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(
		ExeExtensionProperty,
		ExeExtensions)
};
