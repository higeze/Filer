#pragma once
#include "MyPoint.h"
#include <deque>

class CKonamiCommander
{
private:
	std::deque<UINT> m_konami;
	std::deque<UINT> m_keys;
	std::deque<CPoint> m_mousePath;

	static const int OPTION_WIDTH = 12;
	static const int OPTION_HEIGHT = 8;

	static const int OPTION_COUNT = 4;
	static const int MIN_OPTION_DISTANCE = 18;
	static const int BARRIER_CONVEX_WIDTH = 6;
	static const int BARRIER_CONVEX_HEIGHT = 4;
	
	static const int MAX_MOUSEPATH_SIZE = 50;
	static const int MIN_MOUSEPATH_DISTANCE = 5;

	bool m_konamiOn;
	HCURSOR m_hCursor;
	HWND m_hWnd;
public:
	CKonamiCommander():m_konamiOn(false),m_hCursor(NULL)
	{
		m_konami.push_back(VK_UP);
		m_konami.push_back(VK_UP);
		m_konami.push_back(VK_DOWN);
		m_konami.push_back(VK_DOWN);
		m_konami.push_back(VK_LEFT);
		m_konami.push_back(VK_RIGHT);
		m_konami.push_back(VK_LEFT);
		m_konami.push_back(VK_RIGHT);
		m_konami.push_back('B');
		m_konami.push_back('A');
	
	}
	virtual ~CKonamiCommander(){}
	
	void SetHwnd(HWND hWnd) { m_hWnd = hWnd; }

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};