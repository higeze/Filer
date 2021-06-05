#pragma once
#include "GridView.h"
//#include <sigslot/signal.hpp>

struct CellProperty;
struct BackgroundProperty;
struct GridViewProperty;

class CPropertyGridView:public CGridView
{
private:

public:
	CPropertyGridView(CD2DWControl* pParentControl, std::shared_ptr<GridViewProperty> spGridViewProperty);
	virtual ~CPropertyGridView(){}
	sigslot::signal<> PropertyChanged;


	virtual bool HasSheetCell()override{ return true; }
	virtual bool IsVirtualPage()override { return true; }

	virtual void EnsureVisibleCell(const std::shared_ptr<CCell>& pCell){}
	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
	{
		this->PropertyChanged();
		CGridView::OnCellPropertyChanged(pCell, name);
	}

};

