#include "CalendarDialog.h"
#include "D2DWWindow.h"
#include "Button.h"
#include "ButtonProperty.h"
#include "TextBox.h"
#include "TextBoxProperty.h"
#include "CalendarControl.h"
#include "Dispatcher.h"
#include "DialogProperty.h"


CCalendarDialog::CCalendarDialog(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<CalendarControlProperty>& spCalendarProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp)
	:CD2DWDialog(pParentControl, spDialogProp), 
	m_spCalendar(std::make_shared<CCalendarControl>(this, spCalendarProp)),
	m_spYearTextBox(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_spMonthTextBox(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_spButtonClose(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>()))
{
	m_spButtonClose->Command.subscribe([this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
	}, m_spButtonClose);
	m_spButtonClose->Content.set(L"~");
	//m_spCalendar->SelectedYearMonthDay.subscribe([this](auto) 
	//{ 
	//	GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
	//}, m_spCalendar);

	m_spYearTextBox->SetIsEnterText(true);
	m_spMonthTextBox->SetIsEnterText(true);

	m_spCalendar->Year.subscribe([spYearTextBox = m_spYearTextBox](const std::chrono::year& value) {
		if (value.operator int() != _wtoi(spYearTextBox->Text->c_str())) {
			spYearTextBox->Text.set(std::to_wstring(value.operator int()));
		}
	}, m_spYearTextBox);
	m_spYearTextBox->EnterText.subscribe([spCalender = m_spCalendar](auto notify) {
		if (_wtoi(notify.new_items.c_str()) != spCalender->Year->operator int()) {
			spCalender->Year.set(std::chrono::year{_wtoi(notify.new_items.c_str())});
		}
	}, m_spCalendar);

	m_spCalendar->Month.subscribe([spMonthTextBox = m_spMonthTextBox](const std::chrono::month& value) {
		if (value.operator unsigned int() != _wtoi(spMonthTextBox->Text->c_str())) {
			spMonthTextBox->Text.set(std::to_wstring(value.operator unsigned int()));
		}
	}, m_spMonthTextBox);
	m_spMonthTextBox->EnterText.subscribe([spCalender = m_spCalendar](auto notify) {
		if (_wtoi(notify.new_items.c_str()) != spCalender->Month->operator unsigned int()) {
			spCalender->Month.set(std::chrono::month{static_cast<unsigned int>(_wtoi(notify.new_items.c_str()))});
		}
	}, m_spCalendar);


}

CCalendarDialog::~CCalendarDialog() = default;

void CCalendarDialog::Measure(const CSizeF& availableSize)
{
	m_spCalendar->Measure(availableSize);
}

const CSizeF CCalendarDialog::DesiredSize() const
{
	const FLOAT padding = 2.f;
	CSizeF szTextBox = m_spYearTextBox->MeasureSize(L"2023");
	CSizeF szX = m_spYearTextBox->MeasureSize(L"~");
	CSizeF szCalendar = m_spCalendar->DesiredSize();

	return CSizeF(
		(std::max)(szTextBox.width * 2.f + szX.width + padding * 4.f, szCalendar.width + padding * 2.f),
		szTextBox.height + szCalendar.height + padding * padding * 3.f
	);
}

void CCalendarDialog::Arrange(const CRectF& rc)
{
	const FLOAT padding = 2.f;
	CPointF pt(rc.LeftTop().OffsetCopy(CPointF(padding, padding)));
	CSizeF szTextBox = m_spYearTextBox->MeasureSize(L"2023");
	CSizeF szX = m_spYearTextBox->MeasureSize(L"~");
	
	m_spYearTextBox->OnRect(RectEvent(GetWndPtr(), CRectF(pt, szTextBox)));
	m_spMonthTextBox->OnRect(RectEvent(GetWndPtr(), CRectF(pt.OffsetXCopy(szTextBox.width + padding), szTextBox)));
	m_spButtonClose->OnRect(RectEvent(GetWndPtr(), CRectF(rc.right - szX.width - padding, pt.y, rc.right - padding , pt.y + szTextBox.height)));
	pt.y += szTextBox.height + padding;
	m_spCalendar->Arrange(CRectF(pt, m_spCalendar->DesiredSize()));
}

//std::tuple<CRectF, CRectF, CRectF, CRectF> CCalendarDialog::GetRects()
//{
//	CRectF rc = GetRectInWnd();
//	CRectF rcTitle = GetTitleRect();
//	rc.top = rcTitle.bottom;
//
//	CSizeF szTextBox = m_spYearTextBox->MeasureSize(L"2023");
//	
//	CRectF rcYear(rc.left + 5.f, rc.top + 5.f, rc.left + 5.f + szTextBox.width, rc.top + 5.f + szTextBox.height);
//	CRectF rcMonth(rcYear.right + 5.f, rcYear.top, rcYear.right + 5.f + szTextBox.width, rcYear.bottom);
//
//	m_spCalendar->Measure(CSizeF());
//	CSizeF szCalendar = m_spCalendar->DesiredSize();
//	m_spCalendar->Arrange(CRectF(CPointF(rcMonth.left, rcMonth.bottom + 5.f), szCalendar));
//
//
//	CRectF rcBtnClose(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
//	CRectF rcBtnCancel(rcBtnClose.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnClose.left - 5.f, rc.bottom - 5.f);
//	CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);
//	return { rcProgress, rcGrid, rcBtnDo, rcBtnCancel, rcBtnClose };
//}


void CCalendarDialog::OnCreate(const CreateEvt& e)
{
	//Dlg
	CD2DWDialog::OnCreate(e);
	//Size
	//auto [rcProgress, rcGrid, rcBtnDo, rcBtnCancel, rcBtnClose] = GetRects();
	m_spYearTextBox->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_spMonthTextBox->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_spCalendar->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_spButtonClose->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));

	Arrange(GetRectInWnd());

	m_spButtonClose->IsEnabled.set(true);
}

void CCalendarDialog::OnRect(const RectEvent& e)
{
	CD2DWDialog::OnRect(e);

	Arrange(e.Rect);
}

void CCalendarDialog::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spProp->BackgroundFill, GetRectInWnd());

	SendAllReverse(&CD2DWControl::OnPaint, e);

	PaintBorder();

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PopAxisAlignedClip();
}



