#include "UniqueIDFactory.h"


unsigned int CUniqueIDFactory::NewID()
{
	return m_id++;
}

void CUniqueIDFactory::DeleteID(unsigned int id)
{
	//Not implemented
}

