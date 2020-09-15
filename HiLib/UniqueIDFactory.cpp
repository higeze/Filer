#include "UniqueIDFactory.h"


unsigned int CUniqueIDFactory::NewID()
{
	unsigned int id = 0;
	while (true) {
		if (auto iter = m_ids.find(id); iter == m_ids.end()) {
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

