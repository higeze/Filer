#include "FilerWndStatusBar.h"
#include "Debug.h"
#include <psapi.h>
#include <fmt/format.h>
#include "D2DWWindow.h"
#pragma comment(lib, "pdh.lib")

CFilerWndStatusBar::CFilerWndStatusBar(CD2DWControl* pParentControl, const std::shared_ptr<StatusBarProperty>& spStatusBarProp)
	:CStatusBar(pParentControl, spStatusBarProp), 
	m_initialized(false),
	m_hQuery(nullptr),
	m_hCounterCPU(nullptr),
	m_hCounterMemory(nullptr),
	m_hCounterThread(nullptr),
	m_hCounterHandle(nullptr){}

void CFilerWndStatusBar::OnCreate(const CreateEvt& e)
{
	CStatusBar::OnCreate(e);
	m_timer.run([this]()->void { Update(); }, std::chrono::milliseconds(3000));
}


CFilerWndStatusBar::~CFilerWndStatusBar()
{
	m_timer.stop();
	if (m_initialized) {
		::PdhRemoveCounter(m_hCounterCPU);
		::PdhRemoveCounter(m_hCounterMemory);
		::PdhRemoveCounter(m_hCounterThread);
		::PdhRemoveCounter(m_hCounterHandle);
		::PdhCloseQuery(m_hQuery);
	}
}

void CFilerWndStatusBar::Update()
{
	std::lock_guard<std::mutex> lock(m_mtx);
	if (!m_initialized) {
		if (::PdhOpenQuery(NULL, 0, &m_hQuery) == ERROR_SUCCESS &&
		::PdhAddCounter(m_hQuery, L"\\Process(Filer)\\% Processor Time", 0, &m_hCounterCPU) == ERROR_SUCCESS &&
		::PdhAddCounter(m_hQuery, L"\\Process(Filer)\\Private Bytes", 0, &m_hCounterMemory) == ERROR_SUCCESS &&
		::PdhAddCounter(m_hQuery, L"\\Process(Filer)\\Thread Count", 0, &m_hCounterThread) == ERROR_SUCCESS &&
		::PdhAddCounter(m_hQuery, L"\\Process(Filer)\\Handle Count", 0, &m_hCounterHandle) == ERROR_SUCCESS) {
			m_initialized = true;
		} else {
			THROW_FILE_LINE_FUNC;
		}
	}
	try {
		//Memory
		//PROCESS_MEMORY_COUNTERS_EX pmc;
		//GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		//SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
		//m_mem = virtualMemUsedByMe;
		PDH_FMT_COUNTERVALUE    cpuPercent;
		PDH_FMT_COUNTERVALUE    virtualMemory;
		PDH_FMT_COUNTERVALUE    threadCount;
		PDH_FMT_COUNTERVALUE    handleCount;


		if (::PdhCollectQueryData(m_hQuery) == ERROR_SUCCESS) {

			//CPU
			if (PdhGetFormattedCounterValue(m_hCounterCPU, PDH_FMT_DOUBLE, NULL, &cpuPercent) == ERROR_SUCCESS) {
				m_cpu = cpuPercent.doubleValue;
			} else {
				m_cpu = -1;//Error
			}
			//Memory
			if (::PdhGetFormattedCounterValue(m_hCounterMemory, PDH_FMT_LONG, NULL, &virtualMemory) == ERROR_SUCCESS) {
				m_mem = virtualMemory.longValue;
			} else {
				m_mem = -1;//Error
			}
			//Thread
			if (::PdhGetFormattedCounterValue(m_hCounterThread, PDH_FMT_LONG, NULL, &threadCount) == ERROR_SUCCESS) {
				m_threadCount = threadCount.longValue;
			} else {
				m_threadCount = -1;//Error
			}
			//Handle
			if (::PdhGetFormattedCounterValue(m_hCounterHandle, PDH_FMT_LONG, NULL, &handleCount) == ERROR_SUCCESS) {
				m_handleCount = handleCount.longValue;
			} else {
				m_handleCount = -1;//Error
			}
		}
		GetWndPtr()->InvalidateRect(GetWndPtr()->GetDirectPtr()->Dips2Pixels(this->GetRectInWnd()), FALSE);
	}
	catch (...) {
		throw std::exception(FILE_LINE_FUNC);
	}
}

void CFilerWndStatusBar::OnPaint(const PaintEvent& e)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	auto rcPaint = GetRectInWnd();
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spStatusBarProp->BackgroundFill, rcPaint);

	GetWndPtr()->GetDirectPtr()->DrawTextLayout(
		m_spStatusBarProp->Format,
		fmt::format(
		L"CPU:{:.1f}%, PrivateMemory:{:.1f}MB, HandleCount:{}, ThreadCount:{}\t{}",
		m_cpu,
		m_mem / 1024.f / 1024.f,
		m_handleCount,
		m_threadCount,
		m_text),
		rcPaint);
}

void CFilerWndStatusBar::OnSetFocus(const SetFocusEvent& e)
{
	m_timer.run([this]()->void { Update(); }, std::chrono::milliseconds(3000));
}
void CFilerWndStatusBar::OnKillFocus(const KillFocusEvent& e)
{
	m_timer.stop();
}

