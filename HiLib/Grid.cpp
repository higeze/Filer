//#include "Grid.h"
//
//void CGrid::Measure(const CSizeF& availableSize)
//{
//	for (size_t i; i < m_childControls.size(); i++) {
//		m_childControls[i]->Measure(availableSize);
//	}
//	//Column
//	//Auto
//	std::vector<size_t> autos;
//	std::vector<size_t> stars;
//	FLOAT auto_width = 0.f;
//	for (size_t c = 0; c < ColumnDefinitions->size(); c++) {
//		if (*ColumnDefinitions->at(c).Width->Auto) {
//			FLOAT width = 0.f;
//			for (size_t i; i < m_childControls.size(); i++) {
//				if (*m_childControls[i]->GridColumn == c) {
//					width = std::max(width, m_childControls[i]->DesiredSize().width);
//				}
//			}
//			ColumnDefinitions->at(c).Width.get_unconst()->Value.set(width);
//			auto_width += width;
//		}else if(*ColumnDefinitions->at(c).Width->Star) {
//	}
//	//Star
//	FLOAT remain_width = availableSize.width - auto_width;
//	size_t star_count = std::count_if(ColumnDefinitions->cbegin(), ColumnDefinitions->cend(), [](const CColumnDefinition& col) {return *(col.Width->Star); });
//	FLOAT split_width = remain_width / star_count;
//	for (size_t c = 0; c < ColumnDefinitions->size(); c++) {
//		if (*ColumnDefinitions->at(c).Width->Star) {
//			ColumnDefinitions->at(c).Width.get_unconst()->Value.set(split_width);
//		}
//	}
//}
//
//void CGrid::Arrange(const CRectF& rc)
//{
//
//}