#pragma once
#include "Row.h"

class CGridView;
class CShellFile;

class CFileRow:public CRow
{
private:
	std::shared_ptr<CShellFile> m_spFile;
public:
	CFileRow(CSheet* pSheet, std::shared_ptr<CShellFile> spFile);
	virtual ~CFileRow(){}
	std::shared_ptr<CShellFile> GetFilePointer(){return m_spFile;}
	void SetFilePointer(const std::shared_ptr<CShellFile>& spFile){m_spFile = spFile;}
};

