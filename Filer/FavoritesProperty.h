#pragma once
#include "Favorite.h"
#include "KnownFolder.h"
#include "observable.h"

class CFavoritesProperty
{
private:
	std::shared_ptr<observable_vector<std::tuple<std::shared_ptr<CFavorite>>>> m_spFavorites;

public:
	CFavoritesProperty():m_spFavorites(std::make_shared<observable_vector<std::tuple<std::shared_ptr<CFavorite>>>>())
	{
		m_spFavorites->push_back(std::make_tuple(std::make_shared<CFavorite>(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT")));
	};
	~CFavoritesProperty(){};

	std::shared_ptr<observable_vector<std::tuple<std::shared_ptr<CFavorite>>>>& GetFavoritesPtr() { return m_spFavorites; }
	observable_vector<std::tuple<std::shared_ptr<CFavorite>>>& GetFavorites(){return *m_spFavorites;}

    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Favorites", m_spFavorites);
    }
};