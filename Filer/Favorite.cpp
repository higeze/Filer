#include "stdafx.h"
#include "Favorite.h"


CFavorite::CFavorite(void)
	:m_path(), m_shortName(){}

CFavorite::CFavorite(std::wstring path, std::wstring shortName)
	:m_path(path), m_shortName(shortName), m_spFile(){}



