#pragma once
#include <mutex>

extern std::recursive_mutex g_fpdf_mutex;
#define FPDF_LOCK std::lock_guard<std::recursive_mutex> fpdf_lock(g_fpdf_mutex)
