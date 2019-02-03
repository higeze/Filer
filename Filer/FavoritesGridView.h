#pragma once
#include "Gridview.h"
#include "MyFriendSerializer.h"

class CGridViewProperty;
class CFavoritesProperty;
class CShellFile;

class CFavoritesGridView :
	public CGridView
{
private:
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
public:
	boost::signals2::signal<void(std::shared_ptr<CShellFile>&)> FileChosen;

public:
	CFavoritesGridView(std::shared_ptr<CGridViewProperty> spGridViewProrperty, std::shared_ptr<CFavoritesProperty> spFavoritesProp);
	virtual ~CFavoritesGridView(void){}

	std::shared_ptr<CFavoritesProperty>& GetFavoritesProp() { return m_spFavoritesProp; }

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

