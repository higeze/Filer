#include "ColoredTextBox.h"
#include "Scroll.h"

#include "string_extension.h"

/******************/
/* CColoredTextBox */
/******************/

void CColoredTextBox::LoadTextLayoutPtr()
{
	CTextBox::LoadTextLayoutPtr();

	auto pDirect = GetWndPtr()->GetDirectPtr();

	//Syntax
	for (const auto& appearance : GetSyntaxAppearances()) {
		if (!appearance.Regex.empty()) {
			auto brush = pDirect->GetColorBrush(appearance.SyntaxFormat.Color);

			std::wsmatch match;
			auto begin = Text->cbegin();
			auto re = std::wregex(appearance.Regex);//L"/\\*.*?\\*/"
			UINT32 beginPos = 0;
			while (std::regex_search(begin, Text->cend(), match, re)) {
				DWRITE_TEXT_RANGE range{ beginPos + (UINT32)match.position(), (UINT32)match.length() };
				m_pTextLayout->SetDrawingEffect(brush, range);
				if (appearance.SyntaxFormat.IsBold) {
					m_pTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
				}

				begin = match[0].second;
				beginPos = std::distance(Text->cbegin(), begin);
			}
		}
	}			

	//Executable
	m_executableInfos.clear();
	for (const auto& apr : GetExecutableAppearances()) {
		auto brush = pDirect->GetColorBrush(apr.SyntaxFormat.Color);
		std::wsmatch match;
		auto begin = Text->cbegin();
		auto re = std::wregex(apr.Regex);
		UINT32 beginPos = 0;
		while (std::regex_search(begin, Text->cend(), match, re)) {
			DWRITE_TEXT_RANGE range{ beginPos + (UINT32)match.position(), (UINT32)match.length() };
			m_pTextLayout->SetDrawingEffect(brush, range);
		if (apr.SyntaxFormat.IsBold) {
			m_pTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
		}
			m_pTextLayout->SetUnderline(apr.SyntaxFormat.IsUnderline, range);


			m_executableInfos.push_back(ExecutableInfo{ match.str(), range.startPosition, range.length });
			begin = match[0].second;
			beginPos = std::distance(Text->cbegin(), begin);
		}
	}
}

void CColoredTextBox::Normal_LButtonDown(const LButtonDownEvent& e)
{
	auto newPoint = GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);

	if (auto index = GetActualCharPosFromPoint(newPoint)) {
		auto point = GetOriginCharRects()[index.value()].CenterPoint();
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			MoveCaretWithShift(index.value(), point);
			return;
		} else {
			if (::GetAsyncKeyState(VK_CONTROL)) {
				if (auto pos = GetActualCharPosFromPoint(e.PointInWnd)) {
					auto iter = std::find_if(m_executableInfos.begin(), m_executableInfos.end(), [p = static_cast<UINT32>(pos.value())](const auto& info)->bool
					{
						return p >= info.StartPosition && p < info.StartPosition + info.Length;
					});
					if (iter != m_executableInfos.end()) {
						auto exe = iter->Link;
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
			}
			MoveCaret(index.value(), point);
		}
	}
}

void CColoredTextBox::Normal_SetCursor(const SetCursorEvent& e)
{
	CPointF pt = GetWndPtr()->GetCursorPosInWnd();
	if (GetRectInWnd().PtInRect(pt)) {
		if (m_pVScroll->GetIsVisible() && m_pVScroll->GetRectInWnd().PtInRect(pt) ||
			m_pHScroll->GetIsVisible() && m_pHScroll->GetRectInWnd().PtInRect(pt)) {
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			*(e.HandledPtr) = TRUE;
			return;
		} else {
			if (::GetAsyncKeyState(VK_CONTROL)) {
				if (auto pos = GetActualCharPosFromPoint(e.PointInWnd)) {
					auto iter = std::find_if(m_executableInfos.begin(), m_executableInfos.end(), [p = static_cast<UINT32>(pos.value())](const auto& info)->bool
					{
						return p >= info.StartPosition && p < info.StartPosition + info.Length;

					});
					if (iter != m_executableInfos.end()) {
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