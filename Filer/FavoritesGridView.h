#pragma once
#include "BindGridview.h"
#include "Favorite.h"

class CShellFile;

class CFavoritesGridView : public CBindGridView<CFavorite>
{
public:
	std::function<void(const std::shared_ptr<CShellFile>&)> FileChosen;

public:
	CFavoritesGridView( CD2DWControl* pParentControl = nullptr);
	virtual ~CFavoritesGridView(void) = default;

	virtual void Measure(const CSizeF& availableSize) override { CBindGridView::Measure(availableSize); }

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;
	
	void OnCellLButtonDblClk(const CellEventArgs& e);
	void OpenFavorites();
	void UpdateFilter() override {/*Do nothing*/}
	void Reload();

	virtual void MoveRow(int indexTo, typename RowTag::SharedPtr spFrom) override;
};

