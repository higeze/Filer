#pragma once
#include "Direct2DWrite.h"
#include "reactive_property.h"
#include "reactive_string.h"

class CTextLayout;

class CParagraphLayout
{
protected:
	CTextLayout* m_pText;
	mutable CComPtr<IDWriteTextLayout1> m_pTextLayout;
	std::shared_ptr<int> Life;
public:
	reactive_wstring_ptr Text;
public:
	CParagraphLayout(CTextLayout* pText, const std::wstring& text);
	virtual ~CParagraphLayout() = default;
	bool operator==(const CParagraphLayout& rhs) const
	{
		return Text == rhs.Text;
	}

	FLOAT GetWidth() const;
	void SetWidth(const FLOAT& width);
	FLOAT GetHeight() const;
	virtual const CComPtr<IDWriteTextLayout1>& GetTextLayoutPtr() const;
	//CRectF HitTestParagraphPosition(UINT32 position) const;
	CRectF HitTestTextPosition(UINT32 position) const;
	CRectF HitTestCaretTextPosition(UINT32 position) const;
	CRectF HitTestSelectTextPosition(UINT32 position) const;

	std::vector<CRectF> HitTestTextRange(UINT32 position, UINT32 length) const;
	std::vector<CRectF> HitTestSelectTextRange(UINT32 position, UINT32 length) const;
	LONG HitTestPoint(CPointF point) const;
	size_t HitTestCaretPoint(CPointF point) const;
	void Clear();
};


