#pragma once
#include "TextCell.h"

enum class CheckBoxType
{
	Normal,
	ThreeState,
};

enum class CheckBoxState
{
	True,
	False,
	Intermediate,

};

class CCheckBoxCell :public CCell
{
private:
	CheckBoxType m_checkboxType = CheckBoxType::Normal;
public:
	CCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CCheckBoxCell() = default;

	virtual CheckBoxState GetCheckBoxState()const = 0;
	virtual void SetCheckBoxState(const CheckBoxState& checkboxState) = 0;

	CheckBoxType GetCheckBoxType()const { return m_checkboxType; }
	void SetCheckBoxType(const CheckBoxType& checkboxType) { m_checkboxType = checkboxType; }

	static CheckBoxState Str2State(const std::wstring& str);
	static std::wstring State2Str(const CheckBoxState& state);

	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* pDirect) override;
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect) override;

	virtual std::wstring GetString();
	virtual void SetStringCore(const std::wstring& str);

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;

	virtual bool IsComparable()const;
};