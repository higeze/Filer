#include "ToolBar.h"
#include "D2DWWindow.h"


//DesiredSize includes Margin
//RenderSize doesnt include Margin
//ActualSize doesnt include Margin
//Arrange includes Margin

CToolBar::CToolBar(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl)
{
	Margin.set(CRectF());
}

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
			m_size.width += (*iter)->Margin->left;
		} else {
			m_size.width += (std::max)((*std::prev(iter))->Margin->right, (*iter)->Margin->left);
		}
		m_size.width += (*iter)->RenderSize().width;
		if (iter == std::prev(m_childControls.cend())) {
			m_size.width += (*iter)->Margin->right;
		}
	}
	m_size.width += Margin->Width();

	//Measure Height
	m_size.height = (*std::max_element(m_childControls.cbegin(), m_childControls.cend(),
		[](const std::shared_ptr<CD2DWControl>& left, const std::shared_ptr<CD2DWControl>& right) {
		return left->DesiredSize().height < right->DesiredSize().height; }
	))->DesiredSize().height;
	m_size.height += Margin->Height();
}

void CToolBar::Arrange(const CRectF& rc)
{
	FLOAT prev_right = rc.left;
	FLOAT top = rc.top;
	for (auto iter = m_childControls.cbegin(); iter != m_childControls.cend(); ++iter) {
		FLOAT left_offset = 0;
		if (iter == m_childControls.cbegin()) {
			left_offset = 0;
		} else {
			left_offset = (std::max)((*std::prev(iter))->Margin->right, (*iter)->Margin->left) - (*iter)->Margin->left;
		}
		(*iter)->Arrange(CRectF(CPointF(prev_right + left_offset, top), (*iter)->DesiredSize()));
		prev_right = prev_right + left_offset +  (*iter)->Margin->left, (*iter)->RenderSize().width;
	}
}

void CToolBar::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*Background, GetRectInWnd());
	CD2DWControl::OnPaint(e);

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PopAxisAlignedClip();
}


