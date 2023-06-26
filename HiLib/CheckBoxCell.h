#pragma once
#include "TextCell.h"
#include "CheckBoxState.h"

enum class CheckBoxType
{
	Normal,
	ThreeState,
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

	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;
	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override;
	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override;

	virtual std::wstring GetString();
	virtual void SetStringCore(const std::wstring& str);

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnChar(const CharEvent& e) override;

private:
	void NextCheckBoxState();
};