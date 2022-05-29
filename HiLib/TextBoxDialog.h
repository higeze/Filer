#pragma once
#include "D2DWDialog.h"
#include "D2DWWindow.h"
#include "ReactiveProperty.h"

class CTextBlock;
class CTextBox;
struct TextBoxProperty;
class CButton;

class CTextBoxDialog :public CD2DWDialog
{
private:
	std::shared_ptr<CTextBlock> m_spTextBlock;
	std::shared_ptr<CTextBox> m_spTextBox;
	std::shared_ptr<CButton> m_spButtonOK;
	std::shared_ptr<CButton> m_spButtonCancel;
public:
	CTextBoxDialog(
		CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp);
	virtual ~CTextBoxDialog();
	//message, textblock, textbox, ok, cancel
	std::tuple<CRectF, CRectF, CRectF, CRectF> GetRects();
	std::shared_ptr<CTextBlock>& GetTextBlockPtr() { return m_spTextBlock; }
	std::shared_ptr<CTextBox>& GetTextBoxPtr() { return m_spTextBox; }
	std::shared_ptr<CButton>& GetOKButtonPtr() { return m_spButtonOK; }
	std::shared_ptr<CButton>& GetCancelButtonPtr() { return m_spButtonCancel; }


	virtual void OnCreate(const CreateEvt& e);
	virtual void OnRect(const RectEvent& e);
};