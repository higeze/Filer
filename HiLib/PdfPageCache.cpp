#include "PDFPageCache.h"

std::unordered_map<int, PdfBmpInfo>::const_iterator  CPDFPageCache::Find(const int& index)
{
	std::lock_guard<std::mutex> lock(m_mtx);

	auto iter = m_bmpMap.find(index);
	m_pageQue.erase(std::remove(std::begin(m_pageQue), std::end(m_pageQue), index), std::cend(m_pageQue));
	m_pageQue.push_back(index);
	return iter;
}
std::unordered_map<int, PdfBmpInfo>::const_iterator  CPDFPageCache::CEnd() const
{
	return m_bmpMap.cend();
}
std::pair<std::unordered_map<int, PdfBmpInfo>::iterator, bool> CPDFPageCache::InsertOrAssign(const int& index, const PdfBmpInfo& bmp)
{
	std::lock_guard<std::mutex> lock(m_mtx);

	auto pair = m_bmpMap.insert_or_assign(index, bmp);
	m_pageQue.erase(std::remove(std::begin(m_pageQue), std::end(m_pageQue), index), std::cend(m_pageQue));
	m_pageQue.push_back(index);
	return pair;
}
void CPDFPageCache::Prune()
{
	std::lock_guard<std::mutex> lock(m_mtx);

	if (m_pageQue.empty()) { return; }
	long size = 0;
	for (const auto& pair : m_bmpMap) {
		size += static_cast<long>(pair.second.BitmapPtr->GetSize().width * pair.second.BitmapPtr->GetSize().height * 4);
	}

	while (size > m_sizeLimit && m_pageQue.size() > 1) {
		int index = m_pageQue.front();
		size -= static_cast<long>(m_bmpMap[index].BitmapPtr->GetSize().width * m_bmpMap[index].BitmapPtr->GetSize().height * 4);
		m_pageQue.erase(std::remove(std::begin(m_pageQue), std::end(m_pageQue), index), std::cend(m_pageQue));
		m_bmpMap.erase(index);
	}
}