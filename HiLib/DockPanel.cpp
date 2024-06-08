#include "DockPanel.h"
#include "Splitter.h"
#include "D2DWWindow.h"

/**************/
/* CDockPanel */
/**************/
CDockPanel::CDockPanel(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl){}

//void CDockPanel::OnCreate(const CreateEvt& e)
//{
//	CD2DWControl::OnCreate(e);
//
//	CreateEvt evt = CreateEvt(GetWndPtr(), this, CRectF());
//	for (std::shared_ptr<CD2DWControl>& pChild : m_childControls) {
//		if (auto split = std::dynamic_pointer_cast<CSplitter>(pChild)) {
//			split->Value.subscribe([this](auto value) { Arrange(ArrangedRect()); }, shared_from_this());
//		}
//		pChild->OnCreate(evt);
//	}
//	//for (const auto& p : m_ctrl_split_map) {
//	//	p.first->OnCreate(evt);
//	//}
//	//for (const auto& p : m_ctrl_split_map) {
//	//	if (p.second) {
//	//		p.second->OnCreate(evt);
//	//	}
//	//}
//}

void CDockPanel::Measure(const CSizeF& availableSize)
{
	m_size = CSizeF();
	for (const auto& child : m_childControls) {
		child->Measure(availableSize);
		switch (*child->Dock) {
			case DockEnum::Left:
			case DockEnum::Right:
			{
				m_size.width += child->DesiredSize().width;
				m_size.height = (std::max)(m_size.height, child->DesiredSize().height);
				break;
			}
			case DockEnum::Top:
			case DockEnum::Bottom:
			{
				m_size.height += child->DesiredSize().height;
				m_size.width = (std::max)(m_size.width, child->DesiredSize().width);
				break;
			}
			case DockEnum::Fill:
			{
				m_size.width = (std::max)(m_size.width, child->DesiredSize().width);
				m_size.height = (std::max)(m_size.height, child->DesiredSize().height);
				break;
			}
			default:
				THROW_FILE_LINE_FUNC;
				break;
		}
	}
}

void CDockPanel::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);

	CRectF remain = rc;
	FLOAT width = 5;
	std::shared_ptr<CD2DWControl> prev_child;
	for (const auto& child : m_childControls) {
		switch (*child->Dock) {
			case DockEnum::Top:
			{
				child->Arrange(CRectF(remain.left, remain.top, remain.right, remain.top + child->DesiredSize().height));
				remain.top = child->ArrangedRect().bottom;
				//if (auto split = std::dynamic_pointer_cast<CHorizontalSplitter>(child); split && prev_child) {
				//	split->Value.block();
				//	auto minimum = remain.top;
				//	auto maximum = remain.bottom - split->DesiredSize().height;
				//	if (*split->Value < 0) {//Initial
				//		split->Value.set(std::clamp(maximum - child->DesiredSize().height, minimum, maximum));
				//	} else {
				//		if (maximum - minimum != *split->Maximum - *split->Minimum) {
				//			split->Value.set(std::clamp((*split->Value + (maximum - *split->Maximum)), minimum, maximum));
				//		} 
				//	}
				//	split->Minimum.set(minimum);
				//	split->Maximum.set(maximum);
				//	split->Arrange(CRectF(remain.left, *split->Value, remain.right, *split->Value + split->DesiredSize().height));
				//	remain.bottom = split->ArrangedRect().top;
				//	split->Value.unblock();

				//	auto prevRect = prev_child->ArrangedRect();
				//	prevRect.top = split->ArrangedRect().bottom;
				//	prev_child->Arrange(prevRect);
				//}
				break;
			}
			case DockEnum::Bottom:
			{
				if (auto split = std::dynamic_pointer_cast<CHorizontalSplitter>(child); split && prev_child) {
					split->Value.block();
					auto minimum = remain.top;
					auto maximum = prev_child->ArrangedRect().bottom - split->DesiredSize().height;

					if(*split->Maximum < 0 || *split->Minimum < 0){ //Initial
						split->Minimum.set(minimum);
						split->Maximum.set(maximum);
					}

					if (maximum - minimum == *split->Maximum - *split->Minimum){//Size Change
						split->Value.set(std::clamp(*split->Value, minimum, maximum));
					} else {//Normal
						split->Value.set(std::clamp((*split->Value + (maximum - *split->Maximum)), minimum, maximum));
					}
					//if (*split->Value < 0) {//Initial
					//	split->Value.set(std::clamp(maximum, minimum, maximum));
					//} else {
					//	if (maximum - minimum != *split->Maximum - *split->Minimum) {
					//		split->Value.set(std::clamp((*split->Value + (maximum - *split->Maximum)), minimum, maximum));
					//	} 
					//}
					split->Minimum.set(minimum);
					split->Maximum.set(maximum);
					split->Arrange(CRectF(remain.left, *split->Value, remain.right, *split->Value + split->DesiredSize().height));
					remain.bottom = split->ArrangedRect().top;
					split->Value.unblock();

					auto prevRect = prev_child->ArrangedRect();
					prevRect.top = split->ArrangedRect().bottom;
					prev_child->Arrange(prevRect);
				} else {
					child->Arrange(CRectF(remain.left, remain.bottom - child->DesiredSize().height, remain.right, remain.bottom));
					remain.bottom = child->ArrangedRect().top;
				}
				break;
			}
			case DockEnum::Left:
			{
				//TODOTODO
				break;
			}
			case DockEnum::Right:
			{
				if (auto split = std::dynamic_pointer_cast<CVerticalSplitter>(child); split && prev_child) {

					split->Value.block();
					auto minimum = remain.left;
					auto maximum = prev_child->ArrangedRect().right - split->DesiredSize().width;

					if(*split->Maximum < 0 || *split->Minimum < 0){ //Initial
						split->Minimum.set(minimum);
						split->Maximum.set(maximum);
					}

					if (maximum - minimum == *split->Maximum - *split->Minimum){//Size Change
						split->Value.set(std::clamp(*split->Value, minimum, maximum));
					} else {//Normal
						split->Value.set(std::clamp((*split->Value + (maximum - *split->Maximum)), minimum, maximum));
					}

					split->Minimum.set(minimum);
					split->Maximum.set(maximum);
					split->Arrange(CRectF(*split->Value, remain.top, *split->Value + split->DesiredSize().width, remain.bottom));
					remain.right = split->ArrangedRect().left;
					split->Value.unblock();

					auto prevRect = prev_child->ArrangedRect();
					prevRect.left = split->ArrangedRect().right;
					prev_child->Arrange(prevRect);
				} else {
					child->Arrange(CRectF(remain.right - child->DesiredSize().width, remain.top, remain.right, remain.bottom));
					remain.right = child->ArrangedRect().left;
				}
				break;
			}
			case DockEnum::Fill:
				child->Arrange(remain);
				break;
			default:
				THROW_FILE_LINE_FUNC;
				break;
		}
		prev_child = child;
	}
}

