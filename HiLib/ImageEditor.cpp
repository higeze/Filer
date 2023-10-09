#include "ImageEditor.h"
#include "ImageEditorProperty.h"
#include "ImageView.h"
#include "StatusBar.h"
#include "D2DImage.h"
#include "ImageEditorProperty.h"

#include <regex>
#include <nameof/nameof.hpp>


/***************/
/* CImageEditor */
/***************/

CImageEditor::CImageEditor(
	CD2DWControl* pParentControl,
	const std::shared_ptr<ImageEditorProperty>& spProp)
	:CD2DWControl(pParentControl),
	m_spProp(spProp),
	m_spImageView(std::make_shared<CImageView>(this, spProp->ImageViewPropPtr)),
	m_spStatusBar(std::make_shared<CStatusBar>(this, spProp->StatusBarPropPtr)),
	m_spScaleBox(std::make_shared<CTextBox>(this, spProp->TextBoxPropPtr, L"0")),
	m_spPercentBlock(std::make_shared<CTextBlock>(this, spProp->TextBlockPropPtr))
{
	m_spPercentBlock->Text.set(L"%");
}

std::tuple<CRectF, CRectF, CRectF, CRectF> CImageEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	CSizeF percentSize = m_spPercentBlock->MeasureSize();
	CSizeF scaleSize = m_spScaleBox->MeasureSize(L"000.0");
	FLOAT statusHeight = m_spStatusBar->MeasureSize(L"").height;
	FLOAT maxHeight = (std::max)(percentSize.height, scaleSize.height);

	CRectF rcPercent(rcClient.right - percentSize.width,
		rcClient.top + (maxHeight - percentSize.height)*0.5f, 
		rcClient.right, 
		rcClient.top + (maxHeight - percentSize.height)*0.5f+ percentSize.height);
	CRectF rcScale(rcPercent.left - scaleSize.width -2.f,
		rcClient.top+ (maxHeight - scaleSize.height)*0.5f, 
		rcPercent.left -2.f, 
		rcClient.top+ (maxHeight - scaleSize.height)*0.5f + scaleSize.height);

	CRectF rcImage(rcClient.left, rcClient.top + scaleSize.height + 2.f, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcImage.bottom, rcClient.right, rcClient.bottom);

	return { rcScale, rcPercent, rcImage, rcStatus };
}


void CImageEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcScale, rcPercent, rcImage, rcStatus] = GetRects();
	m_spScaleBox->OnCreate(CreateEvt(GetWndPtr(), this, rcScale));
	m_spPercentBlock->OnCreate(CreateEvt(GetWndPtr(), this, rcPercent));
	m_spImageView->OnCreate(CreateEvt(GetWndPtr(), this, rcImage));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));
	m_spImageView->SetIsTabStop(true);

	m_spScaleBox->SetIsEnterText(true);

	auto ratio_to_percent = [](const FLOAT& value)->std::wstring {
		return std::format(L"{:.1f}", std::round(value * 1000.f) / 1000.f * 100.f).c_str();
	};

	auto percent_to_ratio = [](const std::wstring& percent)->FLOAT {
		wchar_t* stopstring;
		return std::round(std::wcstof(percent.c_str(), &stopstring) * 1000.f) / 1000.f / 100.f;
	};

	m_spScaleBox->Text.set(ratio_to_percent(*m_spImageView->Scale));
	m_spImageView->Scale.subscribe(
	[&](const FLOAT& ratio)->void{
		std::wstring percent = ratio_to_percent(ratio);
		if (percent != *m_spScaleBox->Text) {
			m_spScaleBox->Text.set(percent);
		}
	}, shared_from_this());

	m_spScaleBox->EnterText.subscribe(
	[&](auto notify)->void {
		FLOAT ratio = percent_to_ratio(*m_spScaleBox->Text);
		if (ratio != *m_spImageView->Scale) {
			//Validate
			if(ratio == 0.f || m_spImageView->GetMinScale() > ratio || m_spImageView->GetMaxScale() < ratio){ 
				m_spScaleBox->Text.set(ratio_to_percent(*m_spImageView->Scale));
			} else {
				m_spImageView->Scale.set(ratio);
			}
		}
	}, shared_from_this());
}

void CImageEditor::OnPaint(const PaintEvent& e)
{
	m_spScaleBox->OnPaint(e);
	m_spPercentBlock->OnPaint(e);
	m_spImageView->OnPaint(e);
	m_spStatusBar->OnPaint(e);

  using namespace std::chrono;
  const std::chrono::year_month ym(std::chrono::year{2023}, std::chrono::month{10});
  unsigned weekday_offset{ weekday{sys_days{ym/1}}.c_encoding() };  // P1466R3
  unsigned lastday_in_month{ (ym/last).day() };


  std::wstring text =
  L"      " + std::to_wstring(ym.year().operator int()) + std::to_wstring(ym.month().operator size_t()) + L"\n" +
  L"Su Mo Tu We Th Fr Sa\n";
  unsigned wd = 0;
  while (wd++ < weekday_offset) {
      text += L"   ";
  }
  for (unsigned d = 1; d <= lastday_in_month; ++d, ++wd) {
      text += std::to_wstring(d) + (wd % 7 == 0 ? L'\n' : L' ');
  }

  GetWndPtr()->GetDirectPtr()->DrawTextFromPoint(FormatF(), text, GetRectInWnd().LeftTop());
}

void CImageEditor::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	auto [rcScale, rcPercent, rcImage, rcStatus] = GetRects();
	m_spScaleBox->OnRect(RectEvent(GetWndPtr(), rcScale));
	m_spPercentBlock->OnRect(RectEvent(GetWndPtr(), rcPercent));
	m_spImageView->OnRect(RectEvent(GetWndPtr(), rcImage));
	m_spStatusBar->OnRect(RectEvent(GetWndPtr(), rcStatus));
}

void CImageEditor::OnKeyDown(const KeyDownEvent& e)
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

void CImageEditor::Open()
{
	OpenCommand.execute(GetWndPtr()->m_hWnd);
}

void CImageEditor::OpenAs()
{
	OpenAsCommand.execute(GetWndPtr()->m_hWnd);
}

void CImageEditor::Save()
{
	SaveCommand.execute(GetWndPtr()->m_hWnd);
}

void CImageEditor::SaveAs()
{
	SaveAsCommand.execute(GetWndPtr()->m_hWnd);
}

void CImageEditor::Update()
{
}