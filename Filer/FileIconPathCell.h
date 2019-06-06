#pragma once
#include "FileIconNameCell.h"

class CShellFile;

class CFileIconPathCell :public CFileIconNameCell
{
public:
	CFileIconPathCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFileIconPathCell() {}

	virtual std::wstring GetString() override;
	virtual void SetStringCore(const std::wstring& str) override{/*Do Nothing*/}
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override{/*Do Nothing*/}
	virtual void OnLButtonDown(const LButtonDownEvent& e)override{/*Do Nothing*/ }
};