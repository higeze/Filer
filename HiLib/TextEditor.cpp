#include "TextEditor.h"
#include "TextEditorProperty.h"
#include <regex>
#include <nameof/nameof.hpp>

/******************/
/* CEditorTextBox */
/******************/

CEditorTextBox::CEditorTextBox(
	CD2DWControl* pParentControl,
	const std::shared_ptr<TextEditorProperty> pProp,
	const std::wstring& text)
	:CTextBox(pParentControl, pProp, text){}

const CComPtr<IDWriteTextLayout1>& CEditorTextBox::GetTextLayoutPtr()
{
	if (!m_pTextLayout) {
		auto pageRect = GetPageRect();
		auto pDirect = GetWndPtr()->GetDirectPtr();
		auto pRender = pDirect->GetD2DDeviceContext();
		auto pFactory = pDirect->GetDWriteFactory();
		auto size = CSizeF(m_pProp->IsWrap ? (std::max)(0.f, pageRect.Width()) : FLT_MAX, FLT_MAX);

		CComPtr<IDWriteTextLayout> pTextLayout0(nullptr);
		const IID* piid = &__uuidof(IDWriteTextLayout1);
		if (FAILED(pFactory->CreateTextLayout(m_text.c_str(), m_text.size(), pDirect->GetTextFormat(*m_pProp->Format), size.width, size.height, &pTextLayout0)) ||
			FAILED(pTextLayout0->QueryInterface(*piid, (void**)&m_pTextLayout))) {
			throw std::exception(FILE_LINE_FUNC);
		} else {
			//Default set up
			CComPtr<IDWriteTypography> typo;
			pFactory->CreateTypography(&typo);

			DWRITE_FONT_FEATURE feature;
			feature.nameTag = DWRITE_FONT_FEATURE_TAG_STANDARD_LIGATURES;
			feature.parameter = 0;
			typo->AddFontFeature(feature);
			DWRITE_TEXT_RANGE range;
			range.startPosition = 0;
			range.length = m_text.size();
			m_pTextLayout->SetTypography(typo, range);

			m_pTextLayout->SetCharacterSpacing(0.0f, 0.0f, 0.0f, DWRITE_TEXT_RANGE{ 0, m_text.size() });
			m_pTextLayout->SetPairKerning(FALSE, DWRITE_TEXT_RANGE{ 0, m_text.size() });

			//Syntax
			if (auto pTextEditorProp = std::dynamic_pointer_cast<TextEditorProperty>(m_pProp)) {
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
			//auto exePatterns = std::vector<std::wstring>{
			//	LR"((?:[a-zA-Z]:|\\\\[a-zA-Z0-9_.$Åú-]+\\[a-zA-Z0-9_.$Åú-]+)\\(?:[^\\/:*?"<>|\r\n]+\\)*[^\\/:*?"<>|\r\n]*)",
			//	LR"(https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+)"
			//};
			if (auto pTextEditorProp = std::dynamic_pointer_cast<TextEditorProperty>(m_pProp)) {
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

			//Find Highlight
			//m_filter.set(L"Fil");
			//m_text.get().fin
		}
	}
	return m_pTextLayout;
}

void CEditorTextBox::Normal_LButtonDown(const LButtonDownEvent& e)
{
	auto newPoint = GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);

	if (auto index = GetActualCharPosFromPoint(newPoint)) {
		auto point = OriginCharRects()[index.value()].CenterPoint();
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
						SHELLEXECUTEINFO execInfo = {};
						execInfo.cbSize = sizeof(execInfo);
						execInfo.hwnd = GetWndPtr()->m_hWnd;
						execInfo.lpVerb = L"open";
						execInfo.lpFile = exe.c_str();
						execInfo.nShow = SW_SHOWDEFAULT;
						::ShellExecuteEx(&execInfo);			
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

/***************/
/* CTextEditor */
/***************/

CTextEditor::CTextEditor(
	CD2DWControl* pParentControl,
	const std::shared_ptr<TextEditorProperty>& spProp)
	:CD2DWControl(pParentControl),
	m_pProp(spProp),
	m_spTextBox(std::make_shared<CEditorTextBox>(this, spProp, L"")),
	m_spStatusBar(std::make_shared<CStatusBar>(this, spProp->StatusBarPropPtr))
{
	m_spTextBox->SetHasBorder(false);
	m_spTextBox->SetIsScrollable(true);

	m_spStatusBar->GetIsFocusable().set(false);
	
	m_open.Subscribe([this](HWND hWnd)
	{
		m_spTextBox->UpdateAll();
	}, 
	100);
	m_encoding.Subscribe([this](const encoding_type& e)
	{
		m_spStatusBar->SetText(str2wstr(std::string(nameof::nameof_enum(e))));
	});
}



std::tuple<CRectF, CRectF> CTextEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT statusHeight = m_spStatusBar->MeasureSize(GetWndPtr()->GetDirectPtr()).height;
	CRectF rcText(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcText.bottom, rcClient.right, rcClient.bottom);

	if (rcStatus.left > rcStatus.right) {
		auto i = 10;
	}

	return { rcText, rcStatus };
}


void CTextEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcText, rcStatus] = GetRects();
	m_spTextBox->OnCreate(CreateEvt(GetWndPtr(), this, rcText));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));
}

void CTextEditor::OnPaint(const PaintEvent& e)
{
	m_spTextBox->OnPaint(e);
	m_spStatusBar->OnPaint(e);
}

void CTextEditor::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	auto [rcText, rcStatus] = GetRects();
	m_spTextBox->OnRect(RectEvent(GetWndPtr(), rcText));
	m_spStatusBar->OnRect(RectEvent(GetWndPtr(), rcStatus));
	m_spTextBox->UpdateAll();
}


void CTextEditor::OnKeyDown(const KeyDownEvent& e)
{
	bool ctrl = ::GetAsyncKeyState(VK_CONTROL);
	bool shift = ::GetAsyncKeyState(VK_SHIFT);
	switch (e.Char) {
		case 'O':
			if (ctrl && shift) {
				OpenAs();
				*e.HandledPtr = TRUE;
			} else if (ctrl) {
				Open();
				*e.HandledPtr = TRUE;
			}
			break;
		case 'S':
			if (ctrl && shift) {
				SaveAs();
				*e.HandledPtr = TRUE;
			}else if (ctrl) {
				Save();
				*e.HandledPtr = TRUE;
			}
			break;
		default:
			CD2DWControl::OnKeyDown(e);
			break;
	}
}

void CTextEditor::Open()
{
	m_open.Execute(GetWndPtr()->m_hWnd);
}

void CTextEditor::OpenAs()
{
	m_open_as.Execute(GetWndPtr()->m_hWnd);
}

void CTextEditor::Save()
{
	m_save.Execute(GetWndPtr()->m_hWnd);
}

void CTextEditor::SaveAs()
{
	m_save_as.Execute(GetWndPtr()->m_hWnd);
}

void CTextEditor::Update()
{
	m_spTextBox->UpdateAll();
}

