#pragma once
#include "GridView.h"

class CCellProperty;
class CBackgroundProperty;
class CGridViewProperty;

class CPropertyGridView:public CGridView
{
private:

public:
	CPropertyGridView(std::shared_ptr<CGridViewProperty> spGridViewProperty);
	virtual ~CPropertyGridView(){}
	boost::signals2::signal<void()> PropertyChanged;
	virtual void EnsureVisibleCell(const cell_type& pCell){}
	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
	{
		this->PropertyChanged();
		CGridView::OnCellPropertyChanged(pCell, name);
	}
};

