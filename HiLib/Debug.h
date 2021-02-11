#pragma once
#include <Shlwapi.h>
#include <fmt/format.h>

#define FILE_LINE_FUNC fmt::format("File:{}, Line:{}, Func:{}", ::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__).c_str()

#define FALSE_THROW(expression) if(!(expression)){throw std::exception(FILE_LINE_FUNC);}

#define FAILED_THROW(expression) if(FAILED(expression)){throw std::exception(FILE_LINE_FUNC);}

#define THROW_FILE_LINE_FUNC throw std::exception(FILE_LINE_FUNC)