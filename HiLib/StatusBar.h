#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"
#include "Timer.h"
#include <pdh.h>


template<typename T>
std::wstring ConvertCommaSeparatedNumber(T n, int separate_digit)
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

struct StatusBarProperty
{
public:
	d2dw::FormatF Format = d2dw::FormatF();

	d2dw::SolidLine Border = d2dw::SolidLine(230.f / 255.f, 230.f / 255.f, 230.f / 255.f, 1.0f);
	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	d2dw::SolidFill ForegroundFill = d2dw::SolidFill(0.f, 0.f, 0.f, 1.0f);

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Border", Border);
		ar("BackgroundFill", BackgroundFill);
		ar("ForegroundFill", ForegroundFill);
	}
};



namespace d2dw
{

	class CStatusBar: CUIElement
	{
	private:
		CTimer m_timer;
		LONG m_cpu = 0L;
		LONG m_mem = 0L;
		LONG m_threadCount = 0L;

		PDH_HQUERY              m_hQuery;
		PDH_HCOUNTER            m_hCounterCPU;
		PDH_HCOUNTER            m_hCounterMemory;
		PDH_HCOUNTER            m_hCounterThread;

		void Update();

	protected:
		CWnd* m_pWnd;

		CRectF m_rect;

		int m_min = 0;
		int m_max = 100;
		int m_value = 0;

		std::shared_ptr<StatusBarProperty> m_spStatusBarProp;
	public:
		CStatusBar(CWnd* pWnd, const std::shared_ptr<StatusBarProperty>& spStatusBarProp);
		virtual ~CStatusBar();

		CRectF GetRect()const { return m_rect; }
		void SetRect(const CRectF & rect) { m_rect = rect; }
		void SetRect(const FLOAT left, const FLOAT top, const FLOAT right, const FLOAT bottom)
		{
			m_rect.SetRect(left, top, right, bottom);
		}
		CSizeF GetSize()const { return CSizeF(m_rect.Width(), m_rect.Height()); }
		virtual void OnPaint(const PaintEvent & e);
		virtual CSizeF MeasureSize(CDirect2DWrite* pDirect);
	};

}