#include "Performance.h"
#include "Debug.h"

CPerformance::CPerformance()
	:m_initialized(false),
	m_hQuery(nullptr),
	m_hCounterCPU(nullptr),
	m_hCounterMemory(nullptr),
	m_hCounterThread(nullptr),
	m_hCounterHandle(nullptr){}

CPerformance::~CPerformance()
{
	if (m_initialized) {
		::PdhRemoveCounter(m_hCounterCPU);
		::PdhRemoveCounter(m_hCounterMemory);
		::PdhRemoveCounter(m_hCounterThread);
		::PdhRemoveCounter(m_hCounterHandle);
		::PdhCloseQuery(m_hQuery);
	}
}

void CPerformance::Update()
{
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
				m_processorTime = cpuPercent.doubleValue;
			} else {
				m_processorTime = -1;//Error
			}
			//Memory
			if (::PdhGetFormattedCounterValue(m_hCounterMemory, PDH_FMT_LONG, NULL, &virtualMemory) == ERROR_SUCCESS) {
				m_privateBytes = virtualMemory.longValue;
			} else {
				m_privateBytes = -1;//Error
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
	}
	catch (...) {
		throw std::exception(FILE_LINE_FUNC);
	}
}

const std::wstring CPerformance::OutputString()
{
	return std::format(
		L"/* Performance Information */\n"
		L"Process Timer:\t{:.1f}\n"
		L"Private Memory:\t{:.1f}MB\n"
		L"Handle Count:\t{}\n"
		L"Thread Count:\t{}\n",
		m_processorTime,
		m_privateBytes / 1024.f / 1024.f,
		m_handleCount,
		m_threadCount).c_str();
}