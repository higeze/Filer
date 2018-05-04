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
	virtual void CellValueChanged(CellEventArgs& e)
	{
		this->PropertyChanged();
		CGridView::CellValueChanged(e);
	}
};

