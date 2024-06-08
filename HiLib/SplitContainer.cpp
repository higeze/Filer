#include "SplitContainer.h"
#include "Splitter.h"
#include "D2DWWindow.h"

/******************/
/* SplitContainer */
/******************/
CSplitContainer::CSplitContainer(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl)
{
	m_splitter->Value.subscribe([this](auto value) { Arrange(ArrangedRect()); }, Dummy);
}

/***************************/
/* CVerticalSplitContainer */
/***************************/
CVerticalSplitContainer::CVerticalSplitContainer(CD2DWControl* pParentControl)
	:CSplitContainer(pParentControl)
{
	m_splitter = std::make_shared<CVerticalSplitter>(this);
}

void CVerticalSplitContainer::Measure(const CSizeF& availableSize)
{
	m_one->Measure(availableSize);
	m_two->Measure(availableSize);
	m_splitter->Measure(availableSize);
	m_size.width = m_one->DesiredSize().width +  m_two->DesiredSize().width +m_splitter->DesiredSize().width;
	m_size.height = (std::max)({m_one->DesiredSize().height, m_two->DesiredSize().height, m_splitter->DesiredSize().height});
}

void CVerticalSplitContainer::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	
	if (*m_splitter->Value < 0) {//Initial
		m_splitter->Value.set((rc.left, rc.right) * 0.5f);
	} 
	m_splitter->Minimum.set(rc.left);
	m_splitter->Maximum.set(rc.right);
	m_splitter->Arrange(CRectF(*m_splitter->Value, rc.top, *m_splitter->Value + m_splitter->DesiredSize().width, rc.bottom));
	m_one->Arrange(CRectF(rc.left, rc.top, m_splitter->ArrangedRect().left, rc.bottom));
	m_two->Arrange(CRectF(m_splitter->ArrangedRect().right, rc.top, rc.right, rc.bottom));
}

/*****************************/
/* CHorizontalSplitContainer */
/*****************************/
CHorizontalSplitContainer::CHorizontalSplitContainer(CD2DWControl* pParentControl)
	:CSplitContainer(pParentControl)
{
	m_splitter = std::make_shared<CHorizontalSplitter>(this);
}

void CHorizontalSplitContainer::Measure(const CSizeF& availableSize)
{
	m_one->Measure(availableSize);
	m_two->Measure(availableSize);
	m_splitter->Measure(availableSize);
	m_size.width = (std::max)({m_one->DesiredSize().width, m_two->DesiredSize().width, m_splitter->DesiredSize().width});
	m_size.height = m_one->DesiredSize().height + m_two->DesiredSize().height + m_splitter->DesiredSize().height;
}

void CHorizontalSplitContainer::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	
	if (*m_splitter->Value < 0) {//Initial
		m_splitter->Value.set((rc.top + rc.bottom) * 0.5f);
	}

	m_splitter->Minimum.set(rc.top);
	m_splitter->Maximum.set(rc.bottom);
	m_splitter->Arrange(CRectF(rc.left, *m_splitter->Value, rc.right, *m_splitter->Value + m_splitter->DesiredSize().height));
	m_one->Arrange(CRectF(rc.left, rc.top, rc.right, m_splitter->ArrangedRect().top));
	m_two->Arrange(CRectF(rc.left, m_splitter->ArrangedRect().bottom, rc.right, rc.bottom));
}
