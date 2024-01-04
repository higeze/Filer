#pragma once
#include "Favorite.h"
#include "KnownFolder.h"
#include "reactive_vector.h"

class CFavoritesProperty
{
public:
	reactive_vector_ptr<any_tuple> Favorites;

public:
	CFavoritesProperty()
		:Favorites()
	{
		Favorites.push_back(std::make_shared<CFavorite>(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT"));
	};
	~CFavoritesProperty(){};

	friend void to_json(json& j, const CFavoritesProperty& o)
	{
		std::vector<std::shared_ptr<CFavorite>> values;
		std::transform(o.Favorites->cbegin(), o.Favorites->cend(), std::back_inserter(values),
			[](const any_tuple& value) { return value.get<std::shared_ptr<CFavorite>>(); });

		j = json{
			{"Favorites", values}
		};
	}
	friend void from_json(const json& j, CFavoritesProperty& o)
	{
		//std::vector<std::shared_ptr<CFavorite>> values;
		//get_to(j, "Favorites", values);

		//o.Favorites.clear();
		//std::transform(values.begin(), values.end(), std::back_inserter(*o.Favorites.get_unconst()),
		//	[](std::shared_ptr<CFavorite>& value) { return any_tuple(value); });
	}
};

