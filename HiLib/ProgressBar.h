#pragma once
#include "Direct2DWrite.h"
#include "D2DWControl.h"
#include "GridViewProperty.h"

class CProgressBar :public CD2DWControl
{
protected:		
	int m_min = 0;
	int m_max = 100;
	int m_value = 0;

	std::shared_ptr<ProgressProperty> m_spStatusBarProp;
public:
	CProgressBar(CD2DWControl* pParentControl, const std::shared_ptr<ProgressProperty>& spProgressProp);
	virtual ~CProgressBar() = default;

	int GetMax()const { return m_max; }
	void IncrementMax()
	{
		m_max ++;
	}
	void SetMax(int max) 
	{
		if (m_max != max) {
			m_max = max;
			m_value = (std::min)(m_max, m_value);
		}
	}
	int GetMin()const { return m_min; }
	void SetMin(int min)
	{
		if (m_min != min) {
			m_min = min;
			m_value = (std::max)(m_min, m_value);
		}
	}
	int GetValue()const { return m_value; }
	void IncrementValue()
	{
		m_value ++;
	}
	void SetValue(int value)
	{
		if (m_value != value) {
			m_value = std::clamp(value, m_min, m_max);
		}
	}

	//void SetRect(const CRectF& rect) { m_rect = rect; }
	//void SetRect(const FLOAT left, const FLOAT top, const FLOAT right, const FLOAT bottom)
	//{
	//	m_rect.SetRect(left, top, right, bottom);
	//}
	CSizeF GetSize()const { return CSizeF(m_rect.Width(), m_rect.Height()); }
	virtual void OnPaint(const PaintEvent& e);
};
