#pragma once
#include "TextCell.h"
#include "BindRow.h"
#include "CellProperty.h"
#include "GridView.h"
#include "GridView.h"
#include <fmt/format.h>
#include "DriveFolder.h"


template<typename T>
class CDriveSizeCell:public CTextCell
{
private:
	double uli_to_double(const ULARGE_INTEGER& value)
	{
		return static_cast<double>(value.HighPart) * std::pow(2, 32) + static_cast<double>(value.LowPart);
	}

	float uli_to_float(const ULARGE_INTEGER& value)
	{
		return static_cast<float>(value.HighPart) * static_cast<float>(std::pow(2, 32)) + static_cast<float>(value.LowPart);
	}
public:
	virtual const FormatF& GetFormat() const override
	{
		static FormatF value(
			CFontF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9)),
			CColorF(0.0f, 0.0f, 0.0f, 1.0f),
			CAlignmentF(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_TRAILING)); return value;
	}

public:
	CDriveSizeCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CTextCell(pSheet, pRow, pColumn){}

	virtual ~CDriveSizeCell() = default;

	virtual std::wstring GetString() override
	{
		auto spDrive = GetDrivePtr();
		auto [avail, total, free] = spDrive->GetSizes();
		ULARGE_INTEGER used = { .QuadPart = total.QuadPart - free.QuadPart };

		auto double_to_wstring = [](const double& value, const int& precision)->std::wstring
		{
			std::wstringstream ss;
			ss << std::setprecision(precision) << value;
			return ss.str();
		};

		auto get_string = [&](const ULARGE_INTEGER& used, const ULARGE_INTEGER& total, const int& pow, const int& precision, const std::wstring& unit)->std::wstring
		{
			return double_to_wstring(uli_to_double(used) / std::pow(1024, pow) , precision) + unit + L" / " + double_to_wstring(uli_to_double(total) / std::pow(1024, pow), precision) + unit;
		};


		if (total.QuadPart == 0) {
			return L"-";
		} else if (total.QuadPart >= std::pow(1024, 3)) {
			//GB
			return get_string(used, total, 3, 3, L"GB");
		} else if (total.QuadPart >= std::pow(1024, 2)) {
			//MB
			return get_string(used, total, 2, 3, L"MB");
		} else if (total.QuadPart >= std::pow(1024, 1)) {
			//KB
			return get_string(used, total, 1, 3, L"KB");
		} else {
			//B
			return get_string(used, total, 0, 3, L"");
		}
	}

	virtual void PaintBackground(CDirect2DWrite* pDirect, CRectF rcPaint) override
	{
		auto spDrive = GetDrivePtr();
		auto [avail, total, free] = spDrive->GetSizes();
		ULARGE_INTEGER used = { .QuadPart = total.QuadPart - free.QuadPart };

		//Paint Normal
		pDirect->FillSolidRectangle(GetNormalBackground(), rcPaint);

		//Paint Bar
		if (total.QuadPart != 0) {
			float ratio = uli_to_float(used) / uli_to_float(total);
			CRectF rcBar(rcPaint);
			rcBar.right = rcPaint.left + rcPaint.Width() * ratio;
			SolidFill barFill = ratio >= 0.9 ? SolidFill(218.f / 255.f, 38.f / 255.f, 38.f / 255.f, 1.f) : SolidFill(38.f / 255.f, 160.f / 255.f, 218.f / 255.f, 1.f);
			pDirect->FillSolidRectangle(barFill, rcBar);
		}

		//TODO PaintEffect
		//Selected
		if (GetIsSelected() && m_pGrid->GetIsFocused()  /*::GetFocus() == m_pGrid->m_hWnd*/) {
			pDirect->FillSolidRectangle(GetSelectedOverlay(), rcPaint);
		} else if (GetIsSelected()) {
			pDirect->FillSolidRectangle(GetUnfocusSelectedOverlay(), rcPaint);
		}
		//Hot, Pressed
		if (m_state == UIElementState::Hot || m_state == UIElementState::Pressed) {
			pDirect->FillSolidRectangle(GetHotOverlay(), rcPaint);
		}
	}
	
private:
	virtual std::shared_ptr<CDriveFolder> GetDrivePtr()
	{
		if (auto pBindRow = dynamic_cast<CBindRow<T>*>(m_pRow)) {
			auto spFile = pBindRow->GetItem<std::shared_ptr<CShellFile>>();
			if (auto spDrive = std::dynamic_pointer_cast<CDriveFolder>(spFile)) {
				return spDrive;
			}
		}
		return nullptr;
	}
};


