#pragma once
#include "MyFriendSerializer.h"
#include "Favorite.h"
#include "KnownFolder.h"
#include "observable.h"

class CFavoritesProperty
{
private:
	std::shared_ptr<observable_vector<std::shared_ptr<CFavorite>>> m_favorites;

public:
	CFavoritesProperty():m_favorites(std::make_shared<observable_vector<std::shared_ptr<CFavorite>>>())
	{
		m_favorites->push_back(std::make_shared<CFavorite>(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT"));
	};
	~CFavoritesProperty(){};

	std::shared_ptr<observable_vector<std::shared_ptr<CFavorite>>> GetFavorites()const{return m_favorites;}
	void SetFavorites(std::shared_ptr<observable_vector<std::shared_ptr<CFavorite>>>& favorites) { m_favorites = favorites; }

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Favorites", m_favorites);
    }
};