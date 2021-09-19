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
	m_text.Subscribe(
		[this](const NotifyStringChangedEventArgs<wchar_t>& e)->void {
			if (m_changed) { m_changed(e.NewString); }
		}
	);
}

void CCellTextBox::OnClose(const CloseEvent& e)
{
	CTextBox::OnClose(e);

	if (!m_isClosing) {
		m_isClosing = true;
		if (m_final) { m_final(m_text); }
	}
}
bool CCellTextBox::GetIsVisible()const
{
	return m_pCell->GetIsVisible();
}

CRectF CCellTextBox::GetRectInWnd() const
{
	return m_pCell->GetEditRect();
}