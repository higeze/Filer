#include "CheckBoxFilterCell.h"
#include "GridView.h"
#include "GridView.h"
#include "Column.h"
#include "D2DWWindow.h"
#include "Dispatcher.h"

//TOOD why constructor called 3 times
//TODO filter should be OR

CCheckBoxFilterCell::CCheckBoxFilterCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty), m_checkBoxes(), Dummy(std::make_shared<int>(0))
{

	if (pColumn->GetFilter().size() == 1) {
		for (size_t i = 0; i < CHECK_BOX_SIZE; i++) {
			m_checkBoxes.emplace_back(CheckBoxType::ThreeState, Str2State(std::wstring(1, pColumn->GetFilter()[0])));
		}
	} else if (pColumn->GetFilter().size() == 2 && pColumn->GetFilter()[0] == L'-') {
		const std::vector<CheckBoxState> threeStates = {CheckBoxState::False, CheckBoxState::Intermediate, CheckBoxState::True};
		std::vector<CheckBoxState> states;
		std::copy_if(threeStates.cbegin(), threeStates.cend(), std::back_inserter(states),
			[exclude = Str2State(std::wstring(1, pColumn->GetFilter()[1]))](const CheckBoxState& state)->bool {
			return exclude != state;
		});
		m_checkBoxes.emplace_back(CheckBoxType::ThreeState, CheckBoxState(states[0]));
		m_checkBoxes.emplace_back(CheckBoxType::ThreeState, CheckBoxState(states[1]));
		m_checkBoxes.emplace_back(CheckBoxType::ThreeState, CheckBoxState(states[1]));
	} else {
		for (size_t i = 0; i < CHECK_BOX_SIZE; i++) {
			m_checkBoxes.emplace_back(CheckBoxType::ThreeState, static_cast<CheckBoxState>(i + 1));
		}
	}

	auto FilterStr = [this]()->std::wstring {
		const std::vector<CheckBoxState> threeStates = {CheckBoxState::False, CheckBoxState::Intermediate, CheckBoxState::True};
		std::wstring filter;
		std::vector<CheckBoxState> states;
		std::transform(m_checkBoxes.cbegin(), m_checkBoxes.cend(), std::back_inserter(states), [](const CCheckBox& checkBox)->CheckBoxState {
			return *checkBox.State;
		});
		std::sort(states.begin(), states.end());
		states.erase(std::unique(states.begin(), states.end()), states.end());
		switch (states.size()) {
			case 1:
				filter = State2Str(states[0]);
				break;
			case 2:
			{
				std::vector<CheckBoxState> diff;
				std::set_difference(threeStates.cbegin(), threeStates.cend(), states.cbegin(), states.cend(), std::back_inserter(diff));
				filter = L"-" + State2Str(diff[0]);
				break;
			}
			case 3:
			default:
				filter = L"";
				break;
		}
		return filter;
	};

	for (auto& checkBox : m_checkBoxes) {
		checkBox.State.subscribe([this, FilterStr](const CheckBoxState& state) { SetString(FilterStr()); }, Dummy);
	}
}

void CCheckBoxFilterCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	CPointF leftTop = rcPaint.LeftTop();
	for (auto& checkBox : m_checkBoxes) {
		checkBox.Arrange(CRectF(leftTop, checkBox.DesiredSize()));
		checkBox.Render(pDirect);
		leftTop.x += checkBox.DesiredSize().width;
	}
}

CSizeF CCheckBoxFilterCell::MeasureContentSize(CDirect2DWrite* pDirect)
{
	CSizeF sz;
	for (auto& checkBox : m_checkBoxes) {
		checkBox.Measure(CSizeF(16.f, 16.f));
		sz.width += checkBox.DesiredSize().width;
		sz.height = (std::max)(sz.height, checkBox.DesiredSize().height);
	}
	return sz;
}

CSizeF CCheckBoxFilterCell::MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect)
{
	return MeasureContentSize(pDirect);
}

void CCheckBoxFilterCell::OnLButtonDown(const LButtonDownEvent& e)
{
	auto iter = std::find_if(m_checkBoxes.begin(), m_checkBoxes.end(), [ ptInWnd = e.PointInWnd](const auto& checkBox)->bool {
		return checkBox.HitTestCore(ptInWnd) != nullptr;
	});
	if (iter != m_checkBoxes.end()) {
		iter->Toggle();
	}
}

void CCheckBoxFilterCell::SetStringCore(const std::wstring& str)
{
	//Filter cell undo redo is set when Post WM_FILTER
	m_deadlinetimer.run([pWnd = m_pGrid->GetWndPtr(), newString = str, pSheet = m_pGrid, pColumn = m_pColumn]()->void
	{
		pColumn->SetFilter(newString);
		if (auto pGrid = dynamic_cast<CGridView*>(pSheet)) {
			pWnd->GetDispatcherPtr()->PostInvoke([pGrid]()->void { pGrid->OnFilter(); });
		}
	}, std::chrono::milliseconds(200));
}

std::wstring CCheckBoxFilterCell::GetString()
{ 
	return m_pColumn->GetFilter();
}

void CCheckBoxFilterCell::OnPropertyChanged(const wchar_t* name)
{
	if (!_tcsicmp(L"value", name)) {
		//Update valid flag
		m_isFitMeasureValid = false;
		m_isActMeasureValid = false;
	} else if (!_tcsicmp(L"size", name)) {
		m_isActMeasureValid = false;
	}
	//Notify to Row, Column and Sheet
	m_pRow->OnCellPropertyChanged(this, name);
	m_pColumn->OnCellPropertyChanged(this, name);
}