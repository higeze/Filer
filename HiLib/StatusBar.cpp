#include "StatusBar.h"
#include "Debug.h"
#include "ThreadPool.h"
#include <psapi.h>
#include <fmt/format.h>
#pragma comment(lib, "pdh.lib")

namespace d2dw
{
	CStatusBar::CStatusBar(CWnd* pWnd, const std::shared_ptr<StatusBarProperty>& spStatusBarProp)
		:m_pWnd(pWnd), m_spStatusBarProp(spStatusBarProp)
	{
		//Thread
		if (::PdhOpenQuery(NULL, 0, &m_hQuery) == ERROR_SUCCESS &&
			::PdhAddCounter(m_hQuery, L"\\Process(Filer)\\% Processor Time", 0, &m_hCounterCPU) == ERROR_SUCCESS &&
			::PdhAddCounter(m_hQuery, L"\\Process(Filer)\\Private Bytes", 0, &m_hCounterMemory) == ERROR_SUCCESS &&
			::PdhAddCounter(m_hQuery, L"\\Process(Filer)\\Thread Count", 0, &m_hCounterThread) == ERROR_SUCCESS &&
			::PdhAddCounter(m_hQuery, L"\\Process(Filer)\\Handle Count", 0, &m_hCounterHandle) == ERROR_SUCCESS) {
			Update();
			m_timer.run([this]()->void { Update(); }, std::chrono::milliseconds(1000));
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	CStatusBar::~CStatusBar()
	{
		m_timer.stop();
		::PdhRemoveCounter(m_hCounterCPU);
		::PdhRemoveCounter(m_hCounterMemory);
		::PdhRemoveCounter(m_hCounterThread);
		::PdhCloseQuery(m_hQuery);
	}

	void CStatusBar::Update()
	{
		std::lock_guard<std::mutex> lock(m_mtx);
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
			m_pWnd->InvalidateRect(m_pWnd->GetDirectPtr()->Dips2Pixels(this->GetRect()), FALSE);
		}
		catch (...) {

		}
	}

	void CStatusBar::OnPaint(const PaintEvent& e)
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		auto rcPaint = GetRect();
		e.WndPtr->GetDirectPtr()->FillSolidRectangle(m_spStatusBarProp->BackgroundFill, rcPaint);

		e.WndPtr->GetDirectPtr()->DrawTextLayout(m_spStatusBarProp->Format, 
												 fmt::format(
                                                     L"CPU:{:.1f}%, PrivateMemory:{:.1f}MB, HandleCount:{}, ThreadCount:{}, ThreadPool:{}/{}",
                                                     m_cpu,
                                                     m_mem / 1024.f /1024.f,
													 m_handleCount,
													 m_threadCount,
													 CThreadPool::GetInstance()->GetActiveThreadCount(),
													 CThreadPool::GetInstance()->GetTotalTheadCount()),
												 rcPaint);
	}

	CSizeF CStatusBar::MeasureSize(CDirect2DWrite* pDirect)
	{
		//Calc Content Rect
		std::wstring text;
		if (text.empty()) { text = L"a"; }
		return pDirect->CalcTextSize(m_spStatusBarProp->Format, text);
	}
}
