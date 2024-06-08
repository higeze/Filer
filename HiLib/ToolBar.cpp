#include "ToolBar.h"
#include "D2DWWindow.h"


//DesiredSize includes Margin
//RenderSize doesnt include Margin
//RenderSize
//Arrange includes Margin
//GetRectInWnd includes Margin


CToolBar::CToolBar(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl){}

void CToolBar::Measure(const CSizeF& availableSize)
{
	if (m_childControls.empty()) return;
	//Measure Children
	for (auto& spChildControl : m_childControls) {
		spChildControl->Measure(availableSize);
	}
	//Measure Width
	m_size.width = 0.f;
	for (auto iter = m_childControls.cbegin(); iter != m_childControls.cend(); ++iter) {
		if (iter == m_childControls.cbegin()) {
			m_size.width += (*iter)->GetMargin().left;
		} else {
			m_size.width += (std::max)((*std::prev(iter))->GetMargin().right, (*iter)->GetMargin().left);
		}
		m_size.width += (*iter)->RenderSize().width;
		if (iter == std::prev(m_childControls.cend())) {
			m_size.width += (*iter)->GetMargin().right;
		}
	}
	m_size.width += GetMargin().Width();

	//Measure Height
	m_size.height = (*std::max_element(m_childControls.cbegin(), m_childControls.cend(),
		[](const std::shared_ptr<CD2DWControl>& left, const std::shared_ptr<CD2DWControl>& right) {
		return left->DesiredSize().height < right->DesiredSize().height; }
	))->DesiredSize().height;
	m_size.height += GetMargin().Height();
}

void CToolBar::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	FLOAT prev_right = rc.left;
	FLOAT top = rc.top;
	for (auto iter = m_childControls.cbegin(); iter != m_childControls.cend(); ++iter) {
		FLOAT left_offset = 0;
		if (iter == m_childControls.cbegin()) {
			left_offset = 0;
		} else {
			left_offset = (std::max)((*std::prev(iter))->GetMargin().right, (*iter)->GetMargin().left) - (*iter)->GetMargin().left;
		}
		(*iter)->Arrange(CRectF(CPointF(prev_right + left_offset, top), (*iter)->DesiredSize()));
		prev_right = prev_right + left_offset +  (*iter)->GetMargin().left, (*iter)->RenderSize().width;
	}
}

void CToolBar::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);

	CreateEvt evt = CreateEvt(GetWndPtr(), this, CRectF());
	for (std::shared_ptr<CD2DWControl>& pChild : m_childControls) {
		pChild->OnCreate(evt);
	}
}

void CToolBar::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetRectInWnd());
	CD2DWControl::OnPaint(e);

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PopAxisAlignedClip();
}


