#include "CalendarControl.h"
#include "D2DWWindow.h"
#include "YearMonthDay.h"
#include <chrono>

void CCalendarControl::Measure(const CSizeF& availableSize)
{
    using namespace std::chrono;

    if (!m_opt_cell_size.has_value()) {
        CDirect2DWrite* pDirect = GetWndPtr()->GetDirectPtr();
        std::vector<std::wstring> weekday_texts = {L"Su", L"Mo", L"Tu", L"We", L"Th", L"Fr", L"Sa"};
        //Calc max width, height and cell size
        FLOAT text_width = 0;
        FLOAT text_height = 0;
        for (const std::wstring& weekday_text : weekday_texts) {
            CSizeF sz = pDirect->CalcTextSize(m_spProp->Format, weekday_text);
            text_width = (std::max)(text_width, sz.width);
            text_height = (std::max)(text_height, sz.height);
        }
        for (unsigned i = 1; i <= 31; i++) {
            CSizeF sz = pDirect->CalcTextSize(m_spProp->Format, std::to_wstring(i));
            text_width = (std::max)(text_width, sz.width);
            text_height = (std::max)(text_height, sz.height);
        }
        m_opt_cell_size.emplace(
            text_width + m_spProp->Padding.left + m_spProp->Padding.right,
            text_height + +m_spProp->Padding.top + m_spProp->Padding.bottom);
    }

    if (!m_opt_size.has_value()) {
        const std::chrono::year_month ym(*Year, *Month);
        unsigned weekday_offset{ weekday{sys_days{ym/1}}.c_encoding() };  // P1466R3
        unsigned lastday_in_month{ (ym/last).day() };
        m_opt_size.emplace(
            m_opt_cell_size->width * 7.f,
            m_opt_cell_size->height * std::ceilf((weekday_offset + lastday_in_month)/7.f)
        );
    }
}
const CSizeF& CCalendarControl::DesiredSize() const { return m_opt_size.value(); }

void CCalendarControl::Render(CDirect2DWrite* pDirect)
{
    using namespace std::chrono;
    const std::chrono::year_month ym(*Year, *Month);
    const std::chrono::year_month_day today(CYearMonthDay::Today());
    unsigned weekday_offset{ weekday{sys_days{ym/1}}.c_encoding() };  // P1466R3
    unsigned lastday_in_month{ (ym/last).day() };
    std::vector<std::wstring> weekday_texts = {L"Su", L"Mo", L"Tu", L"We", L"Th", L"Fr", L"Sa"};

    //Render
    CRectF cell_rect(m_rect.LeftTop(), m_opt_cell_size.value());
    //Render weekday
    for (const std::wstring& weekday_text : weekday_texts) {
        pDirect->DrawTextLayout(m_spProp->Format, weekday_text, cell_rect);
        cell_rect.OffsetX(m_opt_cell_size->width);
    }
    //Render day
    cell_rect.SetPointSize(
        CPointF(m_rect.LeftTop().x + m_opt_cell_size->width * weekday_offset, m_rect.LeftTop().y + m_opt_cell_size->height),
        m_opt_cell_size.value());
    for (unsigned day = 1, wd = weekday_offset; day <= lastday_in_month; day++, wd++) {
        pDirect->DrawTextLayout(m_spProp->Format, std::to_wstring(day), cell_rect);
        if (wd % 7 == 0) {
            cell_rect.MoveToX(m_rect.LeftTop().x);
            cell_rect.OffsetY(m_opt_cell_size->height);
        } else {
            cell_rect.OffsetX(m_opt_cell_size->width);
        }
    }

  //


  //std::wstring text =
  //L"      " + std::to_wstring(ym.year().operator int()) + std::to_wstring(ym.month().operator size_t()) + L"\n" +
  //L"Su Mo Tu We Th Fr Sa\n";
  //unsigned wd = 0;
  //while (wd++ < weekday_offset) {
  //    text += L"   ";
  //}
  //for (unsigned d = 1; d <= lastday_in_month; ++d, ++wd) {
  //    text += std::to_wstring(d) + (wd % 7 == 0 ? L'\n' : L' ');
  //}

  //GetWndPtr()->GetDirectPtr()->DrawTextFromPoint(FormatF(), text, GetRectInWnd().LeftTop());

}