#pragma once
#include "Gridview.h"
#include "MyFriendSerializer.h"

struct GridViewProperty;
class CFavoritesProperty;
class CShellFile;
class CFilerWnd;

class CFavoritesGridView :
	public CGridView
{
private:
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
	CFilerWnd* m_pFilerWnd;
public:
	boost::signals2::signal<void(std::shared_ptr<CShellFile>&)> FileChosen;

public:
	CFavoritesGridView(CFilerWnd* pWnd, std::shared_ptr<GridViewProperty> spGridViewProrperty, std::shared_ptr<CFavoritesProperty> spFavoritesProp);
	virtual ~CFavoritesGridView(void){}

	std::shared_ptr<CFavoritesProperty>& GetFavoritesProp() { return m_spFavoritesProp; }
	CFilerWnd* GetFilerWndPtr() { return m_pFilerWnd; }

	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	virtual LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void OnCellLButtonDblClk(CellEventArgs& e);
	//virtual void OnContextMenu(ContextMenuEventArgs& e) override;

	void OpenFavorites();

	void RowMoved(CMovedEventArgs<RowTag>& e) override;

	void FilterAll() override {/*Do nothing*/}

	void Reload();

private:
	void InsertDefaultRowColumn();

public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		//ar("FavoritesProperty",m_spFavoritesProp);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		//ar("FavoritesProperty",m_spFavoritesProp);
	}

};

