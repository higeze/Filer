// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//


#pragma once

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 50 //or whatever you need                       
#define BOOST_MPL_LIMIT_MAP_SIZE 50 //or whatever you need        

#define _SCL_SECURE_NO_WARNINGS
#define D_SCL_SECURE_NO_WARNINGS

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// std::make_shared
#define _VARIADIC_MAX	10
// Windows
#include "windows.h"
//#define APSTUDIO_READONLY_SYMBOLS
///////////////////////////////////////////////////////////////////////////////
////
//// Generated from the TEXTINCLUDE 2 resource.
////
//#ifndef APSTUDIO_INVOKED
//#include "targetver.h"
//#endif
//#define APSTUDIO_HIDDEN_SYMBOLS
//#include "windows.h"
////#include <winuser.h>   //WS_EX_, WS_, DS_, ... definitions
////#include <winver.h>    //LVS_, BS_, SS_, ... (control) definitions
////#include <commctrl.h>  //VFT_APP is defined here
////#undef APSTUDIO_HIDDEN_SYMBOLS
///////////////////////////////////////////////////////////////////////////////
//#undef APSTUDIO_READONLY_SYMBOLS

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
//#include <mutex>
//ATL
#include "atlcomcli.h"
// Boost
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/preprocessor.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>


//MultiIndex
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <boost/signals2/signal.hpp>

#include <boost/format.hpp>

#include <boost/filesystem.hpp>

#include <boost/multi_array.hpp>

#include <boost/tokenizer.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

#include "msxml6.tlh"


