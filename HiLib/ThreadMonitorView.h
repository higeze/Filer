#pragma once

#include "ColoredTextBox.h"

class CThreadMonitorView : public CColoredTextBox
{
private:
	CTimer m_update_timer;
public:
	CThreadMonitorView(CD2DWControl* pParentControl)
		:CColoredTextBox(pParentControl, L"") {}
	virtual ~CThreadMonitorView() = default;

	void OnCreate(const CreateEvt& e) override;

};

