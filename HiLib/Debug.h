#pragma once
#include <fmt/format.h>

#define FILE_LINE_FUNC fmt::format("File:{}, Line:{}, Func:{}", ::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__).c_str()
