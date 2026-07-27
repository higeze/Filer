#include "TextLayout.h"
#include "TextBox.h"
#include "ParagraphLayout.h"

/****************/
/* CText Layout */
/****************/

CTextLayout::CTextLayout(CTextBox* pControl)
	:m_pControl(pControl), Life(std::make_shared<int>(1))
{
	m_pControl->Text.subscribe([this](auto notify) { SubscribeTextChange(notify); }, Life);
}

std::wstring CTextLayout::GetParagraphText() const
{
	return std::accumulate(Paragraphs->cbegin(), Paragraphs->cend(), std::wstring(), [](std::wstring str, const std::shared_ptr<CParagraphLayout>& ptr) {return str + *(ptr->Text); });
}

LONG CTextLayout::GetParagraphTextSize() const
{
	return std::accumulate(Paragraphs->cbegin(), Paragraphs->cend(), 0, [](size_t size, const std::shared_ptr<CParagraphLayout>& ptr) {return size + ptr->Text->size(); });
}

FLOAT CTextLayout::GetWidth() const
{
	CSizeF szMin = m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pControl->GetFormat(), L"A");
	FLOAT width = (*std::max_element(Paragraphs->cbegin(), Paragraphs->cend(), [](const std::shared_ptr<CParagraphLayout>& pLhs, const std::shared_ptr<CParagraphLayout>& pRhs)->bool {return pLhs->GetWidth() < pRhs->GetWidth(); }))->GetWidth();
	return (std::max)(szMin.width, width);
}

void CTextLayout::SetWidth(FLOAT width)
{
	if (width > 0) {
		for (auto iter = Paragraphs.get_unconst()->begin(); iter != Paragraphs.get_unconst()->end(); ++iter) {
			(*iter)->SetWidth(width);
		}
	}
}

FLOAT CTextLayout::GetHeight() const
{
	CSizeF szMin = m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pControl->GetFormat(), L"A");
	FLOAT height =  std::accumulate(Paragraphs.cbegin(), Paragraphs.cend(), 0.f, [](FLOAT acc, const std::shared_ptr<CParagraphLayout>& ptr)->FLOAT {return acc + ptr->GetHeight(); });
	return (std::max)(szMin.height, height);
}

//CRectF CText::HitTestParagraphPosition(UINT32 position) const
//{
//	size_t accumulate = 0;
//	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter) {
//		if (position < accumulate + iter->GetText().size()) {
//			auto rect = iter->HitTestTextPosition(position - accumulate);
//			rect.top += accumulate;
//			return rect;
//		}
//		else {
//			accumulate += iter->GetText().size();
//		}
//	}
//	return CRectF();
//}

CRectF CTextLayout::HitTestTextPosition(UINT32 position) const
{
	FLOAT height = 0;
	size_t size = 0;
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter) {
		if (position < size + (*iter)->Text->size()) {
			CRectF rect = (*iter)->HitTestTextPosition(position - size);
			rect.OffsetY(height);
			return rect;
		} else {
			height += (*iter)->GetHeight();
			size += (*iter)->Text->size();
		}
	}
	//TODO HIGH Other CRectF() should be same as this lines
	auto char_size = m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pControl->GetFormat(), L"a");
	return CRectF(0.f, 0.f, char_size.width, char_size.height);
	return CRectF();
}

std::tuple<size_t, CRectF> CTextLayout::HitTestParagraphFirstPosition(UINT32 position) const
{
	FLOAT height = 0;
	size_t size = 0;
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter) {
		if (position < size + (*iter)->Text->size()) {
			CRectF rectFront = (*iter)->HitTestTextPosition(0);
			return { size, rectFront.OffsetYCopy(height) };
		} else {
			height += (*iter)->GetHeight();
			size += (*iter)->Text->size();
		}
	}
	return { 0, CRectF() };
}

std::tuple<size_t, CRectF> CTextLayout::HitTestParagraphLastPosition(UINT32 position) const
{
	FLOAT height = 0;
	size_t size = 0;
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter) {
		if (position < size + (*iter)->Text->size()) {
			CRectF rectBack = (*iter)->HitTestTextPosition((*iter)->Text->size() - 1);
			return { size + (*iter)->Text->size() - 1, rectBack.OffsetYCopy(height) };
		} else {
			height += (*iter)->GetHeight();
			size += (*iter)->Text->size();
		}
	}
	return { 0, CRectF() };
}

CRectF CTextLayout::HitTestCaretTextPosition(UINT32 position) const
{
	//if (m_pControl->Text->size() == 0) {
	//	auto size = m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pControl->GetFormat(), L"a");
	//	return CRectF(0.f, 0.f, size.width, size.height);
	//} else if (m_pControl->Text->size() == (size_t)position) {
	//	auto rect = HitTestTextPosition(position - 1);
	//	rect.left = rect.right;
	//	return rect;
	//} else {
	//	return HitTestTextPosition(position);
	//}

	FLOAT height = 0;
	size_t size = 0;
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter) {
		if ((*iter)->Text->empty() || iter == std::prev(Paragraphs.cend())) {
			CRectF rect = (*iter)->HitTestCaretTextPosition(position - size);
			rect.OffsetY(height);
			return rect;
		} else if (position < size + (*iter)->Text->size()) {
			CRectF rect = (*iter)->HitTestCaretTextPosition(position - size);
			rect.OffsetY(height);
			return rect;
		} else {
			height += (*iter)->GetHeight();
			size += (*iter)->Text->size();
		}
	}
	THROW_FILE_LINE_FUNC;
}

CRectF CTextLayout::HitTestSelectTextPosition(UINT32 position) const
{
	FLOAT height = 0;
	size_t size = 0;
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter) {
		if (position < size + (*iter)->Text->size()) {
			CRectF rect = (*iter)->HitTestSelectTextPosition(position - size);
			rect.OffsetY(height);
			return rect;
		} else {
			height += (*iter)->GetHeight();
			size += (*iter)->Text->size();
		}
	}
	return CRectF();
}

std::vector<CRectF> CTextLayout::HitTestTextRange(UINT32 position, UINT32 length) const
{
	FLOAT height = 0;
	LONG para_first = 0;
	LONG para_last = 0;
	LONG first = position;
	LONG last = std::max(0L, (LONG)position + (LONG)length - 1);
	std::vector<CRectF> rects;
	if (length == 0) { return rects; }
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter)
	{
		para_last = para_first + std::max(0L, (LONG)((*iter)->Text->size() - 1));

		if (para_first <= last && first <= para_last) {
			auto position = std::max(first - para_first, 0L);
			auto length = std::min(last - para_first - position + 1, (LONG)((*iter)->Text->size()));
			auto rcs = (*iter)->HitTestTextRange(position, length);
			std::transform(rcs.cbegin(), rcs.cend(), std::back_inserter(rects), [&](const CRectF& rc) { return rc.OffsetYCopy(height); });
		}
		para_first = para_last + 1;
		height += (*iter)->GetHeight();
	}
	return rects;
}

std::vector<CRectF> CTextLayout::HitTestSelectTextRange(UINT32 position, UINT32 length) const
{
	FLOAT height = 0;
	LONG para_first = 0;
	LONG para_last = 0;
	LONG first = position;
	LONG last = std::max(0L, (LONG)position + (LONG)length - 1);
	std::vector<CRectF> rects;
	if (length == 0) { return rects; }
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter)
	{
		para_last = para_first + std::max(0L, (LONG)((*iter)->Text->size() - 1));

		if (para_first <= last && first <= para_last) {
			auto position = std::max(first - para_first, 0L);
			auto length = std::min(last - para_first - position + 1, (LONG)((*iter)->Text->size()) - position);
			auto rcs = (*iter)->HitTestSelectTextRange(position, length);
			std::transform(rcs.cbegin(), rcs.cend(), std::back_inserter(rects), [&](const CRectF& rc) { return rc.OffsetYCopy(height); });
		}
		para_first = para_last + 1;
		height += (*iter)->GetHeight();
	}
	return rects;
}

LONG CTextLayout::HitTestPoint(CPointF point) const
{
	FLOAT height = 0;
	size_t size = 0;
	auto iter = Paragraphs.cbegin();
	for (; iter != Paragraphs.cend(); ++iter) {
		if (point.y < height + (*iter)->GetHeight()) {
			break;
		} else {
			height += (*iter)->GetHeight();
			size += (*iter)->Text->size();
		}
	}

	if (iter == Paragraphs.cend()) {
		return m_pControl->Text->size();
	} else if (iter == Paragraphs->cbegin() && 0 > point.y) {
		return -1;
	} else {
		auto pos = (*iter)->HitTestPoint(CPointF(point.x, point.y - height));
		pos += size;
		return pos;
	}
}

size_t CTextLayout::HitTestCaretPoint(CPointF point) const
{
	FLOAT top = 0;
	FLOAT bottom = 0;
	size_t size = 0;
	auto iter = Paragraphs.cbegin();
	for (; iter != Paragraphs.cend(); ++iter) {
		bottom += (*iter)->GetHeight();
		if (top <= point.y && point.y < bottom) {
			return (*iter)->HitTestCaretPoint(CPointF(point.x, point.y - top)) + size;
		}
		top = bottom;
		size += (*iter)->Text->size();
	}

	if (iter == Paragraphs.cend()) {
		if (point.y < 0) {
			return 0;
		} else {// point.y >= bottom
			return m_pControl->Text->size() + 1;
		}
	}

}

void CTextLayout::Clear()
{
	Paragraphs.clear();
}

std::optional<size_t> CTextLayout::FindParagraphPosition(size_t position) const
{
	size_t size = 0;
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter) {
		if (position < size + (*iter)->Text->size()) {
			return std::distance(Paragraphs->cbegin(), iter);
		} else {
			size += (*iter)->Text->size();
		}
	}
	return std::nullopt;
}

std::tuple<LONG, LONG> CTextLayout::FindParagraphTextPosition(size_t position) const
{
	size_t size = 0;
	for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend(); ++iter) {
		if (position < size + (*iter)->Text->size()) {
			return { std::distance(Paragraphs->cbegin(), iter), position - size };
		} else {
			size += (*iter)->Text->size();
		}
	}
	return { -1, -1 };
}

void CTextLayout::SubscribeTextChange(const reactive_wstring_ptr::notify_type& notify)
{
	SubscribeTextChangeT<CParagraphLayout>(notify);
}




