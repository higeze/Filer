#include "KonamiCommander.h"
#include "UIElement.h"
#include "UIElement.h"
#include "MyColor.h"
#include "MyBrush.h"
#include "MyPoint.h"
#include "KonamiResource.h"

LRESULT CKonamiCommander::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(m_konamiOn){
		CPoint pt((short)LOWORD(lParam), (short)HIWORD(lParam));

		if(m_konamiOn){
			if(!m_hCursor){
				m_hCursor = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_GRADIUS));
			}
			::SetCursor(m_hCursor);
		}
		CPoint ptCenter(pt.x-16, pt.y);
		if(m_mousePath.empty()){
			m_mousePath.push_back(ptCenter);
		}else{
			//Calculate distance
			auto distanceFromBack = sqrt((pow(m_mousePath.back().x-ptCenter.x, 2) + pow(m_mousePath.back().y-ptCenter.y,2)));
			if(distanceFromBack>=MIN_MOUSEPATH_DISTANCE){
				m_mousePath.push_back(ptCenter);
				if(m_mousePath.size() > MAX_MOUSEPATH_SIZE){
					m_mousePath.pop_front();
				}
			}
		}
	}
	return 0;
}

LRESULT CKonamiCommander::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(m_konamiOn){
		if(!m_hCursor){
			m_hCursor = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_GRADIUS));
		}
		::SetCursor(m_hCursor);
	}
	return 0;
}

LRESULT CKonamiCommander::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_keys.push_back(UINT(wParam));
	if(m_keys.size() > m_konami.size()){
		m_keys.pop_front();
	}
	if(m_konami == m_keys){
		m_konamiOn = true;
	}
	return 0;
}

LRESULT CKonamiCommander::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	if(m_konamiOn && !m_mousePath.empty()){
		auto cr = CColor(RGB(64,170,239));//Summer Sky
		CBrush br(cr);
		CPen pen(PS_SOLID, 1, cr);
		HBRUSH hBr = dc.SelectBrush(br);
		HPEN hPen = dc.SelectPen(pen);
		POINT pt = {0};
		::GetCursorPos(&pt);
		::ScreenToClient(m_hWnd, &pt);
		CPoint ptCur(pt.x-16, pt.y);
		auto size = m_mousePath.size();
		std::vector<CPoint> optionPositions;
		for(int i=size-1,j=0;i>=0 && j<OPTION_COUNT;i--){
			auto distance = sqrt(pow(m_mousePath[i].x - ptCur.x, 2) + pow(m_mousePath[i].y - ptCur.y, 2));
			if(distance>=MIN_OPTION_DISTANCE){
				optionPositions.push_back(m_mousePath[i]);
				ptCur = m_mousePath[i];
				j++;
			}
		}
		for(auto& pt : optionPositions){
			dc.Ellipse(pt.x-OPTION_WIDTH/2,pt.y-OPTION_HEIGHT/2,pt.x+OPTION_WIDTH/2,pt.y+OPTION_HEIGHT/2);
		}
		CPen barrierPen(PS_SOLID, 2, cr);
		dc.SelectPen(barrierPen);
		double PI =  3.14159265;

		std::function<void(CDC*,CPoint,int,int)> paintBarrier = [&](CDC* pDC, CPoint ptCenter, int width, int height)->void{
			std::array<CPoint, 13> barrier;
			int wcos = static_cast<int>(width*cos(45.0 * PI / 180.0));
			int hcos = static_cast<int>(height*cos(45.0 * PI / 180.0));
			barrier[0] = CPoint(ptCenter.x, ptCenter.y-wcos);
			barrier[1] = CPoint(ptCenter.x + hcos, ptCenter.y-wcos-hcos);
			barrier[2] = CPoint(ptCenter.x+wcos+hcos, ptCenter.y-hcos);
			barrier[3] = CPoint(ptCenter.x+wcos, ptCenter.y);
			barrier[4] = CPoint(ptCenter.x+wcos+hcos, ptCenter.y+hcos);
			barrier[5] = CPoint(ptCenter.x+hcos, ptCenter.y+wcos+hcos);
			barrier[6] = CPoint(ptCenter.x, ptCenter.y+wcos);
			barrier[7] = CPoint(ptCenter.x-hcos, ptCenter.y+wcos+hcos);
			barrier[8] = CPoint(ptCenter.x-wcos-hcos, ptCenter.y+hcos);
			barrier[9] = CPoint(ptCenter.x-wcos, ptCenter.y);
			barrier[10] = CPoint(ptCenter.x-wcos-hcos, ptCenter.y-hcos);
			barrier[11] = CPoint(ptCenter.x-hcos, ptCenter.y-wcos-hcos);
			barrier[12] = barrier[0];


			dc.MoveToEx(barrier[0]);
			for(auto i=1;i<13;i++)
			{
				dc.LineTo(barrier[i]);
			}
		};

		paintBarrier(&dc, CPoint(pt.x+5,pt.y-6),BARRIER_CONVEX_WIDTH, BARRIER_CONVEX_HEIGHT);
		paintBarrier(&dc, CPoint(pt.x+5,pt.y+6),BARRIER_CONVEX_WIDTH, BARRIER_CONVEX_HEIGHT);


		dc.SelectBrush(hBr);
		dc.SelectPen(hPen);
	}
	return 0;
}