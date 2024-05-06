#pragma once
#include <mutex>
#include <pdh.h>
#include <psapi.h>
#include <fmt/format.h>
#pragma comment(lib, "pdh.lib")

class CPerformance
{
private:
	bool m_initialized = false;
	std::mutex m_mtx;
	double m_processorTime = 0.f;
	LONG m_privateBytes = 0L;
	LONG m_handleCount = 0L;
	LONG m_threadCount = 0L;

	PDH_HQUERY m_hQuery;
	PDH_HCOUNTER m_hCounterCPU;
	PDH_HCOUNTER m_hCounterMemory;
	PDH_HCOUNTER m_hCounterThread;
	PDH_HCOUNTER m_hCounterHandle;

public:
	CPerformance();
	virtual ~CPerformance();
	void Update();
	const std::wstring OutputString();
};

