#include "UniqueIDFactory.h"


unsigned int CUniqueIDFactory::NewID()
{
	unsigned int id = 0;
	while (true) {
		if (m_ids.find(id) == m_ids.cend()) {
			break;
		} else {
			id++;
		}
	}
	m_ids.insert(id);
	return id++;
}

void CUniqueIDFactory::DeleteID(unsigned int id)
{
	m_ids.erase(id);
}

