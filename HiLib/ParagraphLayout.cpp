#include "ParagraphLayout.h"
#include "TextLayout.h"
#include "Textbox.h"


/*************/
/* Paragraph */
/*************/

CParagraphLayout::CParagraphLayout(CTextLayout* pText, const std::wstring& text)
	:m_pText(pText), Life(std::make_shared<int>(1)), Width(FLT_MAX)
{
	Text.subscribe([this](const reactive_wstring_ptr::notify_type& notify)
		{
			switch (notify.action) {
			case notify_container_changed_action::reset:
			case notify_container_changed_action::insert:
			case notify_container_changed_action::erase:
			case notify_container_changed_action::replace:
			default:
				Clear();
				break;
			}
		}, Life);
	Width.subscribe([this](const FLOAT& value)
		{
			if (GetTextLayoutPtr()) {
				GetTextLayoutPtr()->SetMaxWidth(value);
			}
		}, Life);

	Text.set(text);
}

const CComPtr<IDWriteTextLayout1>& CParagraphLayout::GetTextLayoutPtr() const
{
	if (Text->empty() || *Width <= 0) {

	} else if (!Text->empty() && *Width >= 0 && !m_pTextLayout) {
		auto pDirect = m_pText->m_pControl->GetWndPtr()->GetDirectPtr();
		auto pContext = pDirect->GetD2DDeviceContext();
		auto pFactory = pDirect->GetDWriteFactory();

		const std::wregex re(LR"(\r\n?|\n)");
		std::wstring textWoCRLF = std::regex_replace(*Text, re, L"");

		CComPtr<IDWriteTextLayout> pTextLayout0;
		FAILED_THROW(pFactory->CreateTextLayout(textWoCRLF.c_str(), textWoCRLF.size(), pDirect->GetTextFormat(m_pText->m_pControl->GetFormat()), *Width, FLT_MAX, &pTextLayout0));
		FAILED_THROW(pTextLayout0->QueryInterface(__uuidof(IDWriteTextLayout1), (void**)&m_pTextLayout));

		//Default set up
		CComPtr<IDWriteTypography> typo;
		FAILED_THROW(pFactory->CreateTypography(&typo));

		DWRITE_FONT_FEATURE feature;
		feature.nameTag = DWRITE_FONT_FEATURE_TAG_STANDARD_LIGATURES;
		feature.parameter = 0;
		typo->AddFontFeature(feature);
		DWRITE_TEXT_RANGE range;
		range.startPosition = 0;
		range.length = textWoCRLF.size();
		m_pTextLayout->SetTypography(typo, range);

		m_pTextLayout->SetCharacterSpacing(0.0f, 0.0f, 0.0f, DWRITE_TEXT_RANGE{ 0, textWoCRLF.size() });
		m_pTextLayout->SetPairKerning(FALSE, DWRITE_TEXT_RANGE{ 0, textWoCRLF.size() });
	}
	return m_pTextLayout;
}

CRectF CParagraphLayout::HitTestTextPosition(UINT32 position) const
{
	if (GetTextLayoutPtr()) {
		float x, y;
		DWRITE_HIT_TEST_METRICS tm;
		GetTextLayoutPtr()->HitTestTextPosition(position, false, &x, &y, &tm);
		return CRectF(tm.left, tm.top, tm.left + tm.width, tm.top + tm.height);
	} else {
		auto size = m_pText->m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pText->m_pControl->GetFormat(), L"a");
		return CRectF(0.f, 0.f, size.width, size.height);
	}
}

CRectF CParagraphLayout::HitTestCaretTextPosition(UINT32 position) const
{
	if (GetTextLayoutPtr()) {
		float x, y;
		DWRITE_HIT_TEST_METRICS tm;
		GetTextLayoutPtr()->HitTestTextPosition(position, false, &x, &y, &tm);
		return CRectF(tm.left, tm.top, tm.left + tm.width, tm.top + tm.height);
	} else {
		auto size = m_pText->m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pText->m_pControl->GetFormat(), L"a");
		return CRectF(0.f, 0.f, size.width, size.height);
	}
}

CRectF CParagraphLayout::HitTestSelectTextPosition(UINT32 position) const
{
	auto rect = HitTestTextPosition(position);
	if (Text->at(position) == L'\r' || Text->at(position) == L'\n') {
		auto size = m_pText->m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pText->m_pControl->GetFormat(), L"a");
		rect.right = rect.left + size.width;
	}
	return rect;
}

std::vector<CRectF> CParagraphLayout::HitTestTextRange(UINT32 position, UINT32 length) const
{
	UINT32 actualHitTestMetricsCount;
	GetTextLayoutPtr()->HitTestTextRange(
		position,
		length,
		0, // x
		0, // y
		NULL,
		0, // metrics count
		&actualHitTestMetricsCount);

	std::vector<DWRITE_HIT_TEST_METRICS> hitTestMetrics(actualHitTestMetricsCount);

	GetTextLayoutPtr()->HitTestTextRange(
		position,
		length,
		0, // x
		0, // y
		hitTestMetrics.data(),
		static_cast<UINT32>(hitTestMetrics.size()),
		&actualHitTestMetricsCount);

	std::vector<CRectF> rects;
	for (auto tm : hitTestMetrics) {
		rects.emplace_back(tm.left, tm.top, tm.left + tm.width, tm.top + tm.height);
	}

	return rects;
}

std::vector<CRectF> CParagraphLayout::HitTestSelectTextRange(UINT32 position, UINT32 length) const
{
	auto rects = HitTestTextRange(position, length);
	if (Text->at(position + length - 1) == L'\n') {
		auto size = m_pText->m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pText->m_pControl->GetFormat(), L"a");
		if (rects.size() == 1) {
			rects.at(0).right += size.width;
		} else {
			rects.emplace_back(size);
		}
	}
	//for (size_t i = 0; i < rects.size(); i++) {
	//	if (Text->at(position + i) == '\r' || Text->at(position + i) == '\n') {
	//		auto size = m_pText->m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pText->m_pControl->GetFormat(), L"a");
	//		rects.at(i).right = rects.at(i).left + size.width;
	//	}
	//}
	return rects;
}


LONG CParagraphLayout::HitTestPoint(CPointF point) const
{
	BOOL isTrailingHit, isInside;
	DWRITE_HIT_TEST_METRICS tm;
	GetTextLayoutPtr()->HitTestPoint(point.x, point.y, &isTrailingHit, &isInside, &tm);
	return tm.textPosition;
}

size_t CParagraphLayout::HitTestCaretPoint(CPointF point) const
{
	if (GetTextLayoutPtr()) {
		BOOL isTrailingHit, isInside;
		DWRITE_HIT_TEST_METRICS tm;
		GetTextLayoutPtr()->HitTestPoint(point.x, point.y, &isTrailingHit, &isInside, &tm);

		BOOL isTrailingHitByRect;
		if (/*tm.left <= point.x &&*/ point.x <= tm.left + tm.width / 2) {
			isTrailingHitByRect = FALSE;
		} else if (tm.left + tm.width / 2 <= point.x /*&& point.x <= tm.left + tm.width*/) {
			isTrailingHitByRect = TRUE;
		}



		if (isInside) {
			if (isTrailingHit) {
				return tm.textPosition + 1;
			} else { // isLeadingHit
				return tm.textPosition;
			}
		} else { // is Outside
			if (isTrailingHit) {
				return tm.textPosition + 1;
			} else { // isLeadingHit
				return 0;
			}
		}
	} else {
		return 0;
	}
}

FLOAT CParagraphLayout::GetHeight() const
{
	if (GetTextLayoutPtr()) {
		DWRITE_TEXT_METRICS charMetrics;
		GetTextLayoutPtr()->GetMetrics(&charMetrics);
		return charMetrics.height;
	} else {
		auto size = m_pText->m_pControl->GetWndPtr()->GetDirectPtr()->CalcTextSize(m_pText->m_pControl->GetFormat(), L"a");
		return size.height;
	}
}

void CParagraphLayout::Clear()
{
	m_pTextLayout.Release();
}




