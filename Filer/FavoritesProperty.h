#pragma once
#include "Favorite.h"
#include "KnownFolder.h"
#include "reactive_vector.h"

//class CFavoritesProperty
//{
//public:
//	reactive_vector_ptr<CFavorite> Favorites;
//
//public:
//	CFavoritesProperty()
//		:Favorites()
//	{
//		Favorites.push_back(CFavorite(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT"));
//	};
//	~CFavoritesProperty(){};
//
//    template <class Archive>
//    void serialize(Archive& ar)
//    {
//		ar("Favorites", Favorites);
//    }
//
//	friend void to_json(json& j, const CFavoritesProperty& o)
//	{
//		j = json{
//			{"Favorites", o.Favorites}
//		};
//	}
//	friend void from_json(const json& j, CFavoritesProperty& o)
//	{
//		get_to(j, "Favorites", o.Favorites);
//	}
//};

