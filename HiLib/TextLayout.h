#pragma once
#include "Direct2DWrite.h"
#include "reactive_vector.h"
#include "reactive_string.h"
#include "Textbox.h"
#include "ParagraphLayout.h"

class CTextLayout
{
	friend class CParagraphLayout;
private:
	CTextBox* m_pControl;
	std::shared_ptr<int> Life;

	std::wstring GetParagraphText() const;
	LONG GetParagraphTextSize() const;

public:
	CTextLayout(CTextBox* pControl);
	virtual ~CTextLayout() = default;

	CTextBox* GetTextBoxPtr() { return m_pControl; }
	reactive_vector_ptr<std::shared_ptr<CParagraphLayout>> Paragraphs;

	FLOAT GetWidth() const;
	void SetWidth(FLOAT width);

	FLOAT GetHeight() const;

	CRectF HitTestTextPosition(UINT32 position) const;
	std::tuple<size_t, CRectF> HitTestParagraphFirstPosition(UINT32 position) const;
	std::tuple<size_t, CRectF> HitTestParagraphLastPosition(UINT32 position) const;
	CRectF HitTestCaretTextPosition(UINT32 position) const;
	CRectF HitTestSelectTextPosition(UINT32 position) const;
	std::vector<CRectF> HitTestTextRange(UINT32 position, UINT32 length) const;
	std::vector<CRectF> HitTestSelectTextRange(UINT32 position, UINT32 length) const;
	LONG HitTestPoint(CPointF point) const;
	size_t HitTestCaretPoint(CPointF point) const;

	void Clear();

protected:
	std::optional<size_t> FindParagraphPosition(size_t position) const;
	std::tuple<LONG, LONG> FindParagraphTextPosition(size_t position) const;

	template<typename T>
	std::vector<std::shared_ptr<CParagraphLayout>> split_to_paragraphs(const std::wstring& text)
	{
		std::vector<std::shared_ptr<CParagraphLayout>> paragraphs;
		std::wregex re{ LR"([^\r\n$]*?(\r\n?|\n|$))" };
		for (std::wsregex_iterator re_iter{ text.cbegin(), text.cend(), re }, re_end; re_iter != re_end; ++re_iter) {
			if (re_iter->length()) {
				paragraphs.emplace_back(std::make_shared<T>(this, re_iter->str()));
			}
		}
		return paragraphs;
	}

	template<typename T>
	void sanitize()
	{
		for (auto iter = Paragraphs->cbegin(); iter != Paragraphs->cend();) {
			if (std::next(iter) != Paragraphs->cend() && !(*iter)->Text->empty() && (*iter)->Text->back() != L'\n') {
				auto text = *((*iter)->Text) + *((*std::next(iter))->Text);
				iter = Paragraphs.erase(iter, std::next(iter, 2));
				if (!text.empty()) {
					iter = Paragraphs.insert(iter, std::make_shared<T>(this, text));
					iter++;
				}
			} else {
				iter++;
			}
		}

		if (Paragraphs->size() == 0 || (!Paragraphs->back()->Text->empty() && Paragraphs->back()->Text->back() == L'\n')) {
			Paragraphs.push_back(std::make_shared<T>(this, L""));
		}

		//if (!m_pControl->GetIsWrap()) {
		//	for (auto iter = Paragraphs.get_unconst()->begin(); iter != Paragraphs.get_unconst()->end(); ++iter) {
		//		(*iter)->Width.set(m_width);
		//	}
		//}
	}

	template<typename T>
	void reset(const reactive_wstring_ptr::notify_type& notify)
	{
		Paragraphs.set(split_to_paragraphs<T>(notify.new_items));
		sanitize<T>();
	}

	template<typename T>
	void push_back(const reactive_wstring_ptr::notify_type& notify)
	{
		auto text = *(Paragraphs->back()->Text);
		text.insert(text.cend(), notify.new_items.cbegin(), notify.new_items.cend());
		auto paragraphs = split_to_paragraphs<T>(text);
		auto iter = Paragraphs.erase(std::prev(Paragraphs.cend()));
		if (!paragraphs.empty()) {
			Paragraphs.insert(iter, paragraphs.cbegin(), paragraphs.cend());
		}
		sanitize<T>();
	};

	template<typename T>
	void insert(const reactive_wstring_ptr::notify_type& notify)
	{
		// No Text = Reset
		if (Paragraphs->empty() || GetParagraphTextSize() == 0) {
			reset<T>(notify);
			// Text
		} else {
			// Out of Index = PushBack
			if (notify.new_starting_index < 0 || notify.new_starting_index >= GetParagraphTextSize()) {
				push_back<T>(notify);
				// In Index
			} else {
				auto [para_index, text_index] = FindParagraphTextPosition(notify.new_starting_index);
				auto text = *(Paragraphs.at(para_index)->Text);
				text.insert(text_index, notify.new_items);
				auto paragraphs = split_to_paragraphs<T>(text);
				Paragraphs.erase(Paragraphs.cbegin() + para_index);
				if (!paragraphs.empty()) {
					Paragraphs.insert(Paragraphs.cbegin() + para_index, paragraphs.cbegin(), paragraphs.cend());
				}
			}
		}

		sanitize<T>();
	};

	template<typename T>
	void erase(const reactive_wstring_ptr::notify_type& notify)
	{
		int first_index = notify.old_starting_index;
		int last_index = notify.old_starting_index + notify.old_items.size() - 1;

		int size = 0;
		for (auto iter = Paragraphs.cbegin(); iter != Paragraphs.cend();) {

			if (first_index <= size + static_cast<int>((*iter)->Text->size()) && size <= last_index) {
				auto begin = (std::max)((LONG)(first_index - size), 0L);
				auto end = (std::min)((LONG)(last_index - size), (LONG)(*iter)->Text->size()) + 1;
				auto length = end - begin;
				auto text = *((*iter)->Text);
				text.erase(begin, length);

				size += (*iter)->Text->size();
				iter = Paragraphs.erase(iter);
				if (!text.empty()) {
					iter = Paragraphs.insert(iter, std::make_shared<T>(this, text));
					iter++;
				}
			} else {
				size += (*iter)->Text->size();
				iter++;
			}
		}
		sanitize<T>();
	}

	template<typename T>
	void SubscribeTextChangeT(const reactive_wstring_ptr::notify_type& notify)
	{
		switch (notify.action) {
		case notify_container_changed_action::reset://new,old,0,0
		{
			reset<T>(notify);
			break;
		}
		case notify_container_changed_action::insert://new,null,idx,-1
		{
			insert<T>(notify);
			break;
		}
		case notify_container_changed_action::erase://null,old,-1, idx
		{
			erase<T>(notify);
			break;
		}
		case notify_container_changed_action::replace://new,old,idx,idx
		{
			erase<T>(notify);
			insert<T>(notify);
			break;
		}
		case notify_container_changed_action::push_back:
		{
			push_back<T>(notify);
			break;
		}
		default:
			break;
		}
		auto size1 = m_pControl->Text->size();
		auto size2 = GetParagraphTextSize();
		auto text1 = *m_pControl->Text;
		auto text2 = GetParagraphText();
		assert(m_pControl->Text->size() == GetParagraphTextSize());
		assert(*m_pControl->Text == GetParagraphText());
	}


	virtual void SubscribeTextChange(const reactive_wstring_ptr::notify_type& notify);
};


