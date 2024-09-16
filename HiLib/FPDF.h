#pragma once
#include <fpdfview.h>
#include <fpdf_progressive.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdf_ppo.h>
#include <fpdf_save.h>
#include <fpdf_formfill.h>
#include <fpdf_thumbnail.h>
#pragma comment(lib, "pdfium.dll.lib")

#include <mutex>
#include <optional>

extern std::recursive_mutex g_fpdf_mutex;
#define FPDF_LOCK std::lock_guard<std::recursive_mutex> fpdf_lock(g_fpdf_mutex)
#define FPDF_RETURN_FALSE_IF_CANCEL if (cancel()) { return false; }
