#include "CellTextBox.h"
#include "TextCell.h"

CCellTextBox::CCellTextBox(
	CD2DWControl* pParentControl,
	const std::shared_ptr<TextBoxProperty> pProp,
	const std::wstring& text,
	CTextCell* pCell,
	std::function<void(const std::wstring&)> changed,
	std::function<void(const std::wstring&)> final)
	:CTextBox(pParentControl, pProp, text), m_pCell(pCell),m_changed(changed), m_final(final)
{
	m_acceptsTab = false;
	m_isEnterText = true;
}

void CCellTextBox::OnCreate(const CreateEvt& e)
{
	CTextBox::OnCreate(e);

	Text.subscribe([this](auto notify)->void
	{
		if (m_changed) { m_changed(*Text); }
	}, shared_from_this());
}

void CCellTextBox::OnClose(const CloseEvent& e)
{
	CTextBox::OnClose(e);

	if (!m_isClosing) {
		m_isClosing = true;
		if (m_final) { m_final(*Text); }
	}
}
bool CCellTextBox::GetIsVisible()const
{
	return m_pCell->GetIsVisible();
}

CRectF CCellTextBox::GetRectInWnd() const
{
	//return m_pCell->GetEditRect();
	CRectF rc = m_pCell->GetEditRect();
	//rc.DeflateRect(m_pProp->Line->Width * 0.5f);
	//rc.DeflateRect(*(m_pProp->Padding));

	CSizeF sz = m_pParentControl->GetWndPtr()->GetDirectPtr()->CalcTextSizeWithFixedWidth(*(m_pProp->Format), *Text, rc.Width());

	rc.bottom = (std::max)(rc.bottom, 
		rc.top + sz.height + m_pProp->Line->Width * 0.5f *2.f + m_pProp->Padding->top + m_pProp->Padding->bottom);
	return rc;


}

CEditorCellTextBox::CEditorCellTextBox(
	CD2DWControl* pParentControl,
	const std::shared_ptr<EditorTextBoxProperty> pProp,
	const std::wstring& text,
	CTextCell* pCell,
	std::function<void(const std::wstring&)> changed,
	std::function<void(const std::wstring&)> final)
	:CEditorTextBox(pParentControl, pProp, text), m_pCell(pCell),m_changed(changed), m_final(final)
{
	Text.subscribe([this](auto notify)->void 
	{
		if (m_changed) { m_changed(notify.all_items); }
	}, shared_from_this());
}

void CEditorCellTextBox::OnClose(const CloseEvent& e)
{
	CTextBox::OnClose(e);

	if (!m_isClosing) {
		m_isClosing = true;
		if (m_final) { m_final(*Text); }
	}
}
bool CEditorCellTextBox::GetIsVisible()const
{
	return m_pCell->GetIsVisible();
}

CRectF CEditorCellTextBox::GetRectInWnd() const
{
	//return m_pCell->GetEditRect();

	CRectF rc = m_pCell->GetEditRect();
	rc.DeflateRect(m_pProp->Line->Width * 0.5f);
	rc.DeflateRect(*(m_pProp->Padding));

	CSizeF sz = m_pParentControl->GetWndPtr()->GetDirectPtr()->CalcTextSizeWithFixedWidth(*(m_pProp->Format), *Text, rc.Width());

	rc.bottom = rc.top + sz.height + m_pProp->Line->Width * 0.5f *2.f + m_pProp->Padding->top + m_pProp->Padding->bottom;
	return rc;
}