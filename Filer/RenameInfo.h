#pragma once

struct RenameInfo
{
	auto operator<=>(const RenameInfo&) const = default;

	std::wstring Name;
	std::wstring Ext;
};
