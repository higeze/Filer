#include "SplitContainer.h"
#include "Splitter.h"
#include "D2DWWindow.h"

/******************/
/* SplitContainer */
/******************/
void CSplitContainer::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	m_one->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_two->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_splitter->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_splitter->Value.subscribe([this](auto value) { ArrangeDirty.set(true); }, shared_from_this());
}

/***************************/
/* CVerticalSplitContainer */
/***************************/
CVerticalSplitContainer::CVerticalSplitContainer(CD2DWControl* pParentControl)
	:CSplitContainer(pParentControl)
{
	m_splitter = std::make_shared<CVerticalSplitter>(this);
}

CSizeF CVerticalSplitContainer::MeasureOverride(const CSizeF& availableSize)
{
	m_one->Measure(availableSize);
	m_two->Measure(availableSize);
	m_splitter->Measure(availableSize);
	return CSizeF(m_one->DesiredSize().width +  m_two->DesiredSize().width +m_splitter->DesiredSize().width,
		(std::max)({m_one->DesiredSize().height, m_two->DesiredSize().height, m_splitter->DesiredSize().height}));
}

void CVerticalSplitContainer::ArrangeOverride(const CRectF& rc)
{
	CD2DWControl::ArrangeOverride(rc);
	
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

CSizeF CHorizontalSplitContainer::MeasureOverride(const CSizeF& availableSize)
{
	m_one->Measure(availableSize);
	m_two->Measure(availableSize);
	m_splitter->Measure(availableSize);

	return CSizeF((std::max)({ m_one->DesiredSize().width, m_two->DesiredSize().width, m_splitter->DesiredSize().width }),
		m_one->DesiredSize().height + m_two->DesiredSize().height + m_splitter->DesiredSize().height);
}

void CHorizontalSplitContainer::ArrangeOverride(const CRectF& finalRect)
{
	CD2DWControl::ArrangeOverride(finalRect);
	
	if (*m_splitter->Value < 0) {//Initial
		m_splitter->Value.set((finalRect.top + finalRect.bottom) * 0.5f);
	}

	m_splitter->Minimum.set(finalRect.top);
	m_splitter->Maximum.set(finalRect.bottom);
	m_splitter->Arrange(CRectF(finalRect.left, *m_splitter->Value, finalRect.right, *m_splitter->Value + m_splitter->DesiredSize().height));
	m_one->Arrange(CRectF(finalRect.left, finalRect.top, finalRect.right, m_splitter->ArrangedRect().top));
	m_two->Arrange(CRectF(finalRect.left, m_splitter->ArrangedRect().bottom, finalRect.right, finalRect.bottom));
}
