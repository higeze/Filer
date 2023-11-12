#include "CalendarControl.h"
#include "D2DWWindow.h"
#include "YearMonthDay.h"
#include <chrono>

std::vector<std::wstring> CCalendarControl::s_weekday_texts = {L"Su", L"Mo", L"Tu", L"We", L"Th", L"Fr", L"Sa"};

void CCalendarControl::Measure(const CSizeF& availableSize)
{
    using namespace std::chrono;

    if (!m_opt_cell_size.has_value()) {
        CDirect2DWrite* pDirect = GetWndPtr()->GetDirectPtr();

        //Calc max width, height and cell size
        FLOAT text_width = 0;
        FLOAT text_height = 0;
        for (const std::wstring& weekday_text : s_weekday_texts) {
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
            m_opt_cell_size->height * (1 + std::ceilf((weekday_offset + lastday_in_month)/7.f))
        );
    }
}
const CSizeF& CCalendarControl::DesiredSize() const { return m_opt_size.value(); }

void CCalendarControl::Render(CDirect2DWrite* pDirect)
{
    using namespace std::chrono;
    const std::chrono::year_month ym(*Year, *Month);
    const CYearMonthDay today(CYearMonthDay::Today());
    unsigned weekday_offset{ weekday{sys_days{ym/1}}.c_encoding() };  // P1466R3
    unsigned lastday_in_month{ (ym/last).day() };

    //Render
    CRectF cell_rect(m_rect.LeftTop(), m_opt_cell_size.value());
    //Render weekday
    for (size_t wd = 0; wd < s_weekday_texts.size(); wd++) {
        FormatF format = (wd == 0) ? m_spProp->SundayFormat : (wd == 6) ? m_spProp->SaturdayFormat : m_spProp->Format;
        pDirect->DrawTextLayout(format, s_weekday_texts[wd], cell_rect);
        cell_rect.OffsetX(m_opt_cell_size->width);
    }
    //Render border
    pDirect->DrawSolidLine(m_spProp->Border, CPointF(m_rect.left, cell_rect.bottom), CPointF(cell_rect.left, cell_rect.bottom));
    //Render day
    cell_rect.SetPointSize(
        CPointF(m_rect.LeftTop().x + m_opt_cell_size->width * weekday_offset, m_rect.LeftTop().y + m_opt_cell_size->height),
        m_opt_cell_size.value());
    for (unsigned day = 1, wd = weekday_offset; day <= lastday_in_month; day++, wd++) {
        //Today
        if (CYearMonthDay(Year->operator int(), Month->operator size_t(), day) == today) {
            pDirect->FillSolidRoundedRectangle(m_spProp->TodayFill, cell_rect, 3.f, 3.f);
        } 
        //Selected
        if (CYearMonthDay(Year->operator int(), Month->operator size_t(), day) == *SelectedYearMonthDay){
            pDirect->DrawSolidRoundedRectangle(m_spProp->SelectedLine, cell_rect, 3.f, 3.f);
        }
        //Text
        FormatF format = (wd % 7 == 0) ? m_spProp->SundayFormat : (wd % 7 == 6) ? m_spProp->SaturdayFormat : m_spProp->Format;
        pDirect->DrawTextLayout(format, std::to_wstring(day), cell_rect);
        //Rect
        if (wd % 7 == 6) {
            cell_rect.MoveToX(m_rect.LeftTop().x);
            cell_rect.OffsetY(m_opt_cell_size->height);
        } else {
            cell_rect.OffsetX(m_opt_cell_size->width);
        }
    }
}

void CCalendarControl::OnLButtonClk(const LButtonClkEvent& e)
{
    using namespace std::chrono;
    const std::chrono::year_month ym(*Year, *Month);
    unsigned weekday_offset{ weekday{sys_days{ym/1}}.c_encoding() };  // P1466R3
    unsigned lastday_in_month{ (ym/last).day() };

    CPointF pt(e.PointInWnd - m_rect.LeftTop());
    int x = static_cast<int>(std::floorf(pt.x / m_opt_cell_size->width) + 1.f);
    int y = static_cast<int>(std::floorf(pt.y / m_opt_cell_size->height));

    int day = (y - 1) * 7 - weekday_offset + x;
    if (1 <= day && day <= static_cast<int>(lastday_in_month)) {
        SelectedYearMonthDay.set(CYearMonthDay(Year->operator int(), Month->operator unsigned int(), day));
    }
    Selected.execute(e);
    *e.HandledPtr = TRUE;
}
