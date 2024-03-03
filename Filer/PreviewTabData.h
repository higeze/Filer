#pragma once
#include "TabControl.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"

/******************/
/* PreviewTabData */
/******************/

struct PreviewTabData :public TabData
{
	reactive_property_ptr<CShellFile> Doc;

	PreviewTabData(const std::wstring& path = std::wstring())
		:TabData(), Doc(path){}

	virtual ~PreviewTabData() = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
		PreviewTabData,
		Doc)
};
