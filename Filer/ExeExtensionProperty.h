#pragma once
#include "ReactiveProperty.h"
#include "JsonSerializer.h"

struct ExeExtension
{
	auto operator<=>(const ExeExtension&) const = default;

	std::wstring Name;
	std::wstring Path;
	std::wstring Parameter;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Name", Name);
		ar("Path", Path);
		ar("Parameter", Parameter);
	}
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExeExtension,
		Name,
		Path,
		Parameter)
};

struct ExeExtensionProperty
{
public:
	ReactiveVectorProperty<std::tuple<ExeExtension>> ExeExtensions;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("ExeExtensions", ExeExtensions);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExeExtensionProperty,
		ExeExtensions)
};
