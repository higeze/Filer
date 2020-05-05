#pragma once
#include "BindGridview.h"
#include "Favorite.h"

struct GridViewProperty;
class CFavoritesProperty;
class CShellFile;
class CFilerWnd;

class CFavoritesGridView : public CBindGridView<std::shared_ptr<CFavorite>>
{
private:
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
	CFilerWnd* m_pFilerWnd;
public:
	std::function<void(std::shared_ptr<CShellFile>&)> FileChosen;

public:
	CFavoritesGridView(CFilerWnd* pWnd, std::shared_ptr<GridViewProperty> spGridViewProrperty, std::shared_ptr<CFavoritesProperty> spFavoritesProp);
	virtual ~CFavoritesGridView(void) = default;

	std::shared_ptr<CFavoritesProperty>& GetFavoritesProp() { return m_spFavoritesProp; }
	CFilerWnd* GetFilerWndPtr() { return m_pFilerWnd; }

	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	//virtual LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void OnCellLButtonDblClk(CellEventArgs& e);
	void OpenFavorites();
	void FilterAll() override {/*Do nothing*/}
	void Reload();

public:
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("FavoritesProperty",m_spFavoritesProp);
	}
};

