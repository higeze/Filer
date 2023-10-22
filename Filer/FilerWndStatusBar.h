#pragma once
#include "StatusBar.h"
#include "Timer.h"
#include <pdh.h>

template<typename TRect>
std::wstring ConvertCommaSeparatedNumber(TRect n, int separate_digit)
{
	bool is_minus = n < 0;
	is_minus ? n *= -1 : 0;

	std::wstringstream ss;
	ss << n;
	std::wstring snum = ss.str();
	std::reverse(snum.begin(), snum.end());
	std::wstringstream  ss_csnum;
	for (int i = 0, len = snum.length(); i <= len;) {
		ss_csnum << snum.substr(i, separate_digit);
		if ((i += separate_digit) >= len)
			break;
		ss_csnum << ',';
	}
	if (is_minus) {
		ss_csnum << '-';
	}

	std::wstring cs_num = ss_csnum.str();
	std::reverse(cs_num.begin(), cs_num.end());
	return cs_num;
}

class CFilerWndStatusBar : public CStatusBar
{
private:
	bool m_initialized = false;
	std::mutex m_mtx;
	CTimer m_timer;
	double m_cpu = 0.f;
	LONG m_mem = 0L;
	LONG m_handleCount = 0L;
	LONG m_threadCount = 0L;

	PDH_HQUERY m_hQuery;
	PDH_HCOUNTER m_hCounterCPU;
	PDH_HCOUNTER m_hCounterMemory;
	PDH_HCOUNTER m_hCounterThread;
	PDH_HCOUNTER m_hCounterHandle;

	void Update();

protected:

	int m_min = 0;
	int m_max = 100;
	int m_value = 0;
public:
	CFilerWndStatusBar(CD2DWControl* pParentControl, const std::shared_ptr<StatusBarProperty>& spStatusBarProp);
	virtual ~CFilerWndStatusBar();

	virtual void OnPaint(const PaintEvent& e);

	virtual void OnCreate(const CreateEvt& e)override;
	virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnKillFocus(const KillFocusEvent& e) override;
};
