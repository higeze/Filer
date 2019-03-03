#pragma once
#include "GridView.h"

class CellProperty;
class BackgroundProperty;
struct GridViewProperty;

class CPropertyGridView:public CGridView
{
private:

public:
	CPropertyGridView(std::shared_ptr<GridViewProperty> spGridViewProperty);
	virtual ~CPropertyGridView(){}
	boost::signals2::signal<void()> PropertyChanged;


	virtual bool HasSheetCell()override{ return true; }
	virtual bool IsVirtualPage()override { return true; }

	virtual void EnsureVisibleCell(const std::shared_ptr<CCell>& pCell){}
	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
	{
		this->PropertyChanged();
		CGridView::OnCellPropertyChanged(pCell, name);
	}

};

