#include "ColoredTextBox.h"
#include "ColoredTextLayout.h"
#include "Scroll.h"

#include "string_extension.h"

/******************/
/* CColoredTextBox */
/******************/

CColoredTextBox::CColoredTextBox()
	: CTextBox()
{
	m_pTextLayout = std::make_unique<CColoredTextLayout>(this);
}

CColoredTextBox::CColoredTextBox(
	CD2DWControl* pParentControl,
	const std::wstring& text) 
	: CTextBox(pParentControl, text)
{
	m_pTextLayout = std::make_unique<CColoredTextLayout>(this);
}

CColoredTextBox::CColoredTextBox(
	CD2DWControl* pParentControl,
	std::unique_ptr<CVScroll>&& pVScroll,
	std::unique_ptr<CHScroll>&& pHScroll,
	const std::wstring& text)
	:CTextBox(m_pParentControl,
		std::forward<std::unique_ptr<CVScroll>>(pVScroll),
		std::forward<std::unique_ptr<CHScroll>>(pHScroll), text)
{
	m_pTextLayout = std::make_unique<CColoredTextLayout>(this);
}

void CColoredTextBox::Normal_LButtonDown(const LButtonDownEvent& e)
{
	auto index = HitTestCaretPoint(e.PointInWnd);
	auto caret_point = m_pTextLayout->HitTestTextPosition(index).CenterPoint();

	if (GetKeyState(VK_SHIFT) & 0x8000) {
		MoveCaretWithShift(index, caret_point);
		return;
	} else if (::GetAsyncKeyState(VK_CONTROL)) {

		if (auto p = dynamic_cast<CColoredTextLayout*>(m_pTextLayout.get())) {
			auto exeInfo = p->HitTestExecutableInfo(index);

			if (exeInfo.has_value()) {
				auto exe = exeInfo->Link;
				exe = ((exe.front() == L'\"') ? L"" : L"\"") + boost::algorithm::trim_copy(exe) + ((exe.back() == L'\"') ? L"" : L"\"");
				SHELLEXECUTEINFO sei = { 0 };
				sei.cbSize = sizeof(sei);
				sei.hwnd = GetWndPtr()->m_hWnd;
				sei.lpVerb = nullptr;
				sei.lpFile = exe.c_str();
				sei.nShow = SW_SHOWDEFAULT;
				::ShellExecuteEx(&sei);
				return;
			}
		}

		MoveCaret(index, caret_point);
	} else {
		MoveCaret(index, caret_point);
	}
}

void CColoredTextBox::Normal_SetCursor(const SetCursorEvent& e)
{
	auto index = HitTestCaretPoint(e.PointInWnd);

	CPointF pt = GetWndPtr()->GetCursorPosInWnd();
	if (GetRectInWnd().PtInRect(pt)) {
		if (m_pVScroll->GetIsVisible() && m_pVScroll->GetRectInWnd().PtInRect(pt) ||
			m_pHScroll->GetIsVisible() && m_pHScroll->GetRectInWnd().PtInRect(pt)) {
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			*(e.HandledPtr) = TRUE;
			return;
		} else {
			if (::GetAsyncKeyState(VK_CONTROL)) {
				if (auto p = dynamic_cast<CColoredTextLayout*>(m_pTextLayout.get())) {
					auto exeInfo = p->HitTestExecutableInfo(index);

					if (exeInfo.has_value()) {
						::SetCursor(::LoadCursor(NULL, IDC_HAND));
						*(e.HandledPtr) = TRUE;
						return;
					}

				}
			}
		}
		::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
		*(e.HandledPtr) = TRUE;
	}
}