#include "EditorTextBox.h"
#include "EditorScroll.h"
#include "EditorProperty.h"
#include "Editor.h"
#include "string_extension.h"

/******************/
/* CEditorTextBox */
/******************/

CEditorTextBox::CEditorTextBox(
	CD2DWControl* pParentControl,
	const std::shared_ptr<EditorTextBoxProperty>& pProp,
	const std::wstring& text)
	:CTextBox(pParentControl, 
		std::make_unique<CEditorVScroll>(this, std::static_pointer_cast<EditorScrollProperty>(pProp->VScrollPropPtr)), 
		std::make_unique<CHScroll>(this, pProp->HScrollPropPtr),
		pProp,
		text)
{
	m_pVScroll->ScrollChanged.connect([this]() { ClearHighliteRects(); });
}

void CEditorTextBox::PaintHighlite(const PaintEvent& e)
{
	//Draw Highlite
	for (auto rc : GetHighliteRects()) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(
			std::static_pointer_cast<EditorTextBoxProperty>(m_pProp)->FindHighliteFill,
			rc);
	}
}

void CEditorTextBox::LoadTextLayoutPtr()
{
	CTextBox::LoadTextLayoutPtr();

	auto pDirect = GetWndPtr()->GetDirectPtr();

	//Syntax
	if (auto pTextEditorProp = std::dynamic_pointer_cast<EditorTextBoxProperty>(m_pProp)) {
		for (const auto& tuple : pTextEditorProp->SyntaxAppearances.get()) {
			auto appearance = std::get<0>(tuple);
			if (!appearance.Regex.empty()) {
				auto brush = pDirect->GetColorBrush(appearance.SyntaxFormat.Color);

				std::wsmatch match;
				auto begin = m_text.cbegin();
				auto re = std::wregex(appearance.Regex);//L"/\\*.*?\\*/"
				UINT32 beginPos = 0;
				while (std::regex_search(begin, m_text.cend(), match, re)) {
					DWRITE_TEXT_RANGE range{ beginPos + (UINT32)match.position(), (UINT32)match.length() };
					m_pTextLayout->SetDrawingEffect(brush, range);
					if (appearance.SyntaxFormat.IsBold) {
						m_pTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
					}

					begin = match[0].second;
					beginPos = std::distance(m_text.cbegin(), begin);
				}
			}
		}
	}
	//https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+
	//(?:[a-zA-Z]:|\\\\[a-zA-Z0-9_.$Åú-]+\\[a-zA-Z0-9_.$Åú-]+)\\(?:[^\\/:*?"<>|\r\n]+\\)*[^\\/:*?"<>|\r\n]* 
	// 
				

	//Executable
	// 
	//auto exePatterns = std::vector<std::wstring>{
	//	LR"((?:[a-zA-Z]:|\\\\[a-zA-Z0-9_.$Åú-]+\\[a-zA-Z0-9_.$Åú-]+)\\(?:[^\\/:*?"<>|\r\n]+\\)*[^\\/:*?"<>|\r\n]*)",
	//	LR"(https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+)"
	//};
	if (auto pTextEditorProp = std::dynamic_pointer_cast<EditorTextBoxProperty>(m_pProp)) {
		m_executableInfos.clear();
		for (const auto& apr : pTextEditorProp->ExecutableAppearances) {
			auto brush = pDirect->GetColorBrush(apr.SyntaxFormat.Color);
			std::wsmatch match;
			auto begin = m_text.cbegin();
			auto re = std::wregex(apr.Regex);
			UINT32 beginPos = 0;
			while (std::regex_search(begin, m_text.cend(), match, re)) {
				DWRITE_TEXT_RANGE range{ beginPos + (UINT32)match.position(), (UINT32)match.length() };
				m_pTextLayout->SetDrawingEffect(brush, range);
			if (apr.SyntaxFormat.IsBold) {
				m_pTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
			}
				m_pTextLayout->SetUnderline(apr.SyntaxFormat.IsUnderline, range);


				m_executableInfos.push_back(ExecutableInfo{ match.str(), range.startPosition, range.length });
				begin = match[0].second;
				beginPos = std::distance(m_text.cbegin(), begin);
			}
		}
	}
}

void CEditorTextBox::LoadHighliteRects()
{
	//Find Highlight
	m_optHighliteRects = std::vector<CRectF>();
	const auto pEditor = static_cast<CEditor*>(GetParentControlPtr());
	const auto spFilter = pEditor->GetFilterBoxPtr();
	const auto& find = spFilter->GetText().get();

	auto subStrSize = find.size();
	auto pos = m_text.get() | find_ignorecase(find);
	const auto& actualCharRects = GetActualCharRects();
 
	while (subStrSize != 0 && pos != std::wstring::npos) {
		m_optHighliteRects->emplace_back(
			actualCharRects[pos].left,
			actualCharRects[pos].top,
			actualCharRects[pos + subStrSize - 1].right,
			actualCharRects[pos].bottom
		);
		pos = m_text.get() | find_ignorecase(find, pos + subStrSize);
	}
}

void CEditorTextBox::ClearHighliteRects()
{
	m_optHighliteRects = std::nullopt;
	static_cast<CEditorVScroll*>(m_pVScroll.get())->ClearHighliteRects();
}

void CEditorTextBox::Normal_LButtonDown(const LButtonDownEvent& e)
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

void CEditorTextBox::Normal_SetCursor(const SetCursorEvent& e)
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

void CEditorTextBox::UpdateAll()
{
	CTextBox::UpdateAll();
	ClearHighliteRects();
}