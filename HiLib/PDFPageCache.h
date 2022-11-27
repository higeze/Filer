#pragma once
#include<unordered_map>
#include "PDFPage.h"

class CPDFPageCache
{
private:
	std::mutex m_mtx;
	std::unordered_map<int, PdfBmpInfo> m_bmpMap;
	std::vector<int> m_pageQue;

	LONG m_sizeLimit = 64*1024*1024;
public:
	CPDFPageCache() {}

	std::unordered_map<int, PdfBmpInfo>::const_iterator Find(const int& index);
	std::unordered_map<int, PdfBmpInfo>::const_iterator  CEnd() const;
	std::pair<std::unordered_map<int, PdfBmpInfo>::iterator, bool> InsertOrAssign(const int& index, const PdfBmpInfo& bmp);
	void Prune();
};
