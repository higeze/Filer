#pragma once
#include "Gridview.h"

class CGridViewProperty;
class CFavoritesProperty;
class CShellFolder;

class CFavoritesGridView :
	public CGridView
{
private:
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;

public:
	CFavoritesGridView(std::shared_ptr<CGridViewProperty> spGridViewProrperty, std::shared_ptr<CFavoritesProperty> spFavoritesProp);
	virtual ~CFavoritesGridView(void){}

	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	void OnCellLButtonDblClk(CellEventArgs& e);
	//virtual void OnContextMenu(ContextMenuEventArgs& e) override;

		boost::signals2::signal<void(std::shared_ptr<CShellFolder>&)> FolderChoiced;
	void OpenFavorites();

	void RowMoved(CMovedEventArgs<RowTag>& e) override;

private:
	void InsertDefaultRowColumn();

};

