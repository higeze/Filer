// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _SCL_SECURE_NO_WARNINGS
#define D_SCL_SECURE_NO_WARNINGS
#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// std::make_shared
#define _VARIADIC_MAX	10
// Windows
#include "windows.h"

//#define USE_PYTHON_EXTENSION

#ifdef USE_PYTHON_EXTENSION
#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>
//Need to link python lib
#endif

#include <CommDlg.h>
#include <uxtheme.h>
#include <vssym32.h>
#pragma comment(lib, "Uxtheme.lib")
#include <WinGDI.h>
#pragma comment(lib, "msimg32.lib")
#include <OleIdl.h>
#include <shlwapi.h>
#pragma comment (lib, "shlwapi.lib")

#include <ShellAPI.h>
#include <shlobj.h>
#pragma comment(lib, "Ole32.lib")
// C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <string.h>
// C++
#include <numeric>
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <deque>
#include <vector>
#include <algorithm>
#include <functional>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <tuple>
#include <typeinfo>
#include <unordered_set>
#include <array>
#include <exception>
#include <thread>
//ATL
#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>
// Boost
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
//#include <boost/preprocessor.hpp>
#include <boost/algorithm/string.hpp>
//#include <boost/signals2/signal.hpp>
//#include <boost/multi_array.hpp>
#include <boost/tokenizer.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

#include "msxml6.tlh"

#undef min
#undef max
