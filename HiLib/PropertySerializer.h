#pragma once

#include "MyMPL.h"
#include "MyDC.h"
#include "MyString.h"
#include "PropertyGridView.h"

#include "Row.h"
#include "ParentRowHeaderColumn.h"
#include "PropertyColumn.h"

#include "CellProperty.h"
#include "Sheet.h"
#include "SheetCell.h"
#include "ColorCell.h"
#include "FontCell.h"
#include "BoolCell.h"
#include "MySerializer.h"

#include "PropertySheetCell.h"
#include <type_traits>
#include "SerializerEnableIf.h"

class CCellSerializer
{
private:
	std::weak_ptr<CSheet> m_pSheet;
	CDC* m_pDC;
	std::shared_ptr<CCellProperty> m_spPropSheetCellHeader;
	std::shared_ptr<CCellProperty> m_spPropSheetCellFilter;
	std::shared_ptr<CCellProperty> m_spPropSheetCellCell;

public:
	CCellSerializer(
		std::shared_ptr<CSheet> pSheet,
		std::shared_ptr<CCellProperty> spPropSheetCellHeader,
		std::shared_ptr<CCellProperty> spPropSheetCellFilter,
		std::shared_ptr<CCellProperty> spPropSheetCellCell)
		:m_pSheet(pSheet),
		m_spPropSheetCellHeader(spPropSheetCellHeader),
		m_spPropSheetCellFilter(spPropSheetCellFilter),
		m_spPropSheetCellCell(spPropSheetCellCell){}
	virtual ~CCellSerializer(){}

	template<class T>
	void Serialize(std::shared_ptr<CPropertyGridView> pGrid,const TCHAR* lpszRootName,T& t,ENABLE_IF_SERIALIZE)
	{
		//Serialize
		t.serialize(*this);
	}
	template<class T>
	void Serialize(std::shared_ptr<CPropertyGridView> pGrid,const TCHAR* lpszRootName,T& t,ENABLE_IF_SAVE_LOAD)
	{
		//Serialize
		t.save(*this);
	}

	template<class T>
	void Serialize(std::shared_ptr<CPropertyGridView> pGrid,const TCHAR* lpszRootName,T* pT)
	{
		Serialize(pGrid,lpszRootName,*pT);
	}

	template<class T>
	void Serialize(std::shared_ptr<CPropertyGridView> pGrid,const TCHAR* lpszRootName,std::shared_ptr<T> pT)
	{
		Serialize(pGrid,lpszRootName,*pT);
	}

	template<class char_type,class T>
	void operator()(const char_type* lpszName,T& t)
	{

		if(auto spSheet=m_pSheet.lock()){
			std::shared_ptr<CRow> pRow;
			std::shared_ptr<CColumn> pColProperty;
			std::shared_ptr<CColumn> pColValue;	
			if(auto pSheet=std::dynamic_pointer_cast<CGridView>(spSheet)){
				if(pSheet->Empty()){
					auto pRowHeader=std::make_shared<CParentHeaderRow>(pSheet.get());
					auto pRowFilter=std::make_shared<CParentRow>(pSheet.get());
					auto pColHeader=std::make_shared<CParentRowHeaderColumn>(pSheet.get());
					pSheet->SetNameHeaderRowPtr(pRowHeader);
					pSheet->SetFilterRowPtr(pRowFilter);
					pSheet->SetHeaderColumnPtr(pColHeader);
					pSheet->InsertRowNotify(CRow::kMinIndex,pRowHeader);
					pSheet->InsertRowNotify(CRow::kMinIndex, pRowFilter);
					pSheet->InsertColumnNotify(CColumn::kMinIndex, pColHeader);

					pRow=std::make_shared<CParentRow>(pSheet.get());
					pSheet->InsertRowNotify(CRow::kMaxIndex, pRow);
			
					pColProperty=std::make_shared<CParentPropertyNameColumn>(pSheet.get());
					pColValue=std::make_shared<CParentPropertyValueColumn>(pSheet.get());

					pSheet->InsertColumnNotify(CColumn::kMaxIndex, pColProperty);
					pSheet->InsertColumnNotify(CColumn::kMaxIndex, pColValue);
				}else{
					pRow=std::make_shared<CParentRow>(pSheet.get());
					pSheet->InsertRowNotify(CRow::kMaxIndex, pRow);
					pColProperty=pSheet->Index2Pointer<ColTag, AllTag>(0);
					pColValue=pSheet->Index2Pointer<ColTag, AllTag>(1);
				}
			
			}else if(auto pSheet=std::dynamic_pointer_cast<CSheetCell>(spSheet)){

				if(pSheet->Empty()){
					auto pChildRowHeader=std::make_shared<CChildHeaderRow>(pSheet.get());
					pSheet->SetHeaderRowPtr(pChildRowHeader);
					pSheet->InsertRowNotify(CRow::kMinIndex,pChildRowHeader);

					pRow=std::make_shared<CChildRow>(pSheet.get());
					pSheet->InsertRowNotify(CColumn::kMaxIndex,pRow);
			
					pColProperty=std::make_shared<CChildPropertyNameColumn>(pSheet.get());
					pColValue=std::make_shared<CChildPropertyValueColumn>(pSheet.get());

					pSheet->InsertColumnNotify(CColumn::kMaxIndex, pColProperty);
					pSheet->InsertColumnNotify(CColumn::kMaxIndex, pColValue);
				}else{
					pRow=std::make_shared<CChildRow>(pSheet.get());
					pSheet->InsertRowNotify(CRow::kMaxIndex, pRow);
					pColProperty=pSheet->Index2Pointer<ColTag, AllTag>(0);
					pColValue=pSheet->Index2Pointer<ColTag, AllTag>(1);
				}
			}

			//spSheet->UpdateRowVisibleDictionary();
			//spSheet->UpdateColumnVisibleDictionary();

			pColProperty->Cell(pRow.get())->SetStringCore(std::wstring(lpszName,(lpszName+strlen(lpszName))));
			SerializeValue(t,pRow.get(),pColValue.get());	
		}else{
			return;
		}
	}

	//For base
	template<class T>
	void SerializeValue(T& t,CRow* pRow,CColumn* pCol,ENABLE_IF_DEFAULT)
	{
		pCol->Cell(pRow)->SetStringCore(boost::lexical_cast<std::wstring>(t));
	}

	//For std::string boost::lexical cast couldn't cast std::string to std::wstring
	void SerializeValue(std::string& t,CRow* pRow,CColumn* pCol)
	{
		pCol->Cell(pRow)->SetStringCore(str2wstr(t));
	}

	//For Color
	void SerializeValue(CColor& t,CRow* pRow,CColumn* pCol)
	{
		if(auto spSheet=m_pSheet.lock()){
			pCol->Cell(pRow)=std::make_shared<CColorCell>(
				spSheet.get(),
				pRow,
				pCol,
				spSheet->GetCellProperty(),
				t);
		}
	}
	//For Font
	void SerializeValue(CFont& t,CRow* pRow,CColumn* pCol)
	{
		if(auto spSheet=m_pSheet.lock()){
			pCol->Cell(pRow)=std::make_shared<CFontCell>(
				spSheet.get(),
				pRow,
				pCol,
				spSheet->GetCellProperty(),
				t);
		}
	}

	//For bool
	void SerializeValue(bool& t,CRow* pRow,CColumn* pCol)
	{
		if(auto spSheet=m_pSheet.lock()){
			pCol->Cell(pRow)=std::make_shared<CBoolCell>(
				spSheet.get(),
				pRow,
				pCol,
				spSheet->GetCellProperty(),
				t);
		}
	}

	//For enum
	template<class T>
	void SerializeValue(T& t,CRow* pRow,CColumn* pCol,ENABLE_IF_ENUM)
	{
		pCol->Cell(pRow)->SetStringCore(boost::lexical_cast<std::wstring>(static_cast<int>(t)));
	}
	
	//For serialize
	template<class T>
	void SerializeValue(T& t,CRow* pRow,CColumn* pCol,ENABLE_IF_SERIALIZE)
	{
		if(auto spSheet=m_pSheet.lock()){
			std::shared_ptr<CSheetCell> spSheetCell(
				std::make_shared<CSheetCell>(
				spSheet.get(),
				pRow,
				pCol,
				spSheet->GetCellProperty(),
				m_spPropSheetCellHeader,
				m_spPropSheetCellFilter,
				m_spPropSheetCellCell));

			pCol->Cell(pRow)=spSheetCell;

			t.serialize(CCellSerializer(std::dynamic_pointer_cast<CSheet>(spSheetCell),m_spPropSheetCellHeader,m_spPropSheetCellFilter,m_spPropSheetCellCell));
			spSheetCell->UpdateAll();
		}
	}

	//For save load
	template<class T>
	void SerializeValue(T& t, CRow* pRow, CColumn* pCol, ENABLE_IF_SAVE_LOAD)
	{
		if (auto spSheet = m_pSheet.lock()) {
			std::shared_ptr<CSheetCell> spSheetCell(
				std::make_shared<CSheetCell>(
					spSheet.get(),
					pRow,
					pCol,
					spSheet->GetCellProperty(),
					m_spPropSheetCellHeader,
					m_spPropSheetCellFilter,
					m_spPropSheetCellCell));

			pCol->Cell(pRow) = spSheetCell;

			t.save(CCellSerializer(std::dynamic_pointer_cast<CSheet>(spSheetCell), m_spPropSheetCellHeader, m_spPropSheetCellFilter, m_spPropSheetCellCell));
			spSheetCell->UpdateAll();
		}
	}

	//For ptr
	template<class T>
	void SerializeValue(T& t, CRow* pRow, CColumn* pCol, ENABLE_IF_PTR)
	{
		SerializeValue(*t, pRow, pCol);
	}

	//For vector
	template<class T>
	void SerializeValue(std::vector<T>& t,CRow* pRow,CColumn* pCol)
	{
		if(auto spSheet=m_pSheet.lock()){
			std::shared_ptr<CRow> spRow;
			std::shared_ptr<CColumn> pColIndex;
			std::shared_ptr<CColumn> pColValue;	
			auto spSheetCell = std::make_shared<CPropertySheetCell<T>>(
				spSheet.get(),
				pRow,
				pCol,
				spSheet->GetCellProperty(),
				m_spPropSheetCellHeader,
				m_spPropSheetCellFilter,
				m_spPropSheetCellCell);

			auto pChildRowHeader=std::make_shared<CChildHeaderRow>(spSheetCell.get());
			spSheetCell->SetHeaderRowPtr(pChildRowHeader);
			spSheetCell->InsertRowNotify(CRow::kMinIndex, pChildRowHeader);
			
			pColIndex=std::make_shared<CChildPropertyIndexColumn>(spSheetCell.get());
			pColValue=std::make_shared<CChildPropertyValueColumn>(spSheetCell.get());

			spSheetCell->InsertColumnNotify(CColumn::kMaxIndex, pColIndex);
			spSheetCell->InsertColumnNotify(CColumn::kMaxIndex, pColValue);
			CCellSerializer serializer(spSheetCell,m_spPropSheetCellHeader,m_spPropSheetCellFilter,m_spPropSheetCellCell);
			for(auto& val : t){
				spRow=std::make_shared<CChildRow>(spSheetCell.get());
				spSheetCell->InsertRowNotify(CRow::kMaxIndex, spRow);
				//pColProperty=pSheet->AllColumn(0);
				pColValue=spSheetCell->Index2Pointer<ColTag, AllTag>(1);
				serializer.SerializeValue(val,spRow.get(),pColValue.get());
			}
			CSheet::Cell(pRow, pCol) = spSheetCell;
			spSheetCell->UpdateAll();
		}
	}
	//For shared_ptr
	template<class T>
	void SerializeValue(std::shared_ptr<T>& t,CRow* pRow,CColumn* pCol)
	{
		SerializeValue(*t,pRow,pCol);
	}
};


class CCellDeserializer
{
private:
	std::weak_ptr<CSheet> m_pSheet;
	std::unordered_set<CCell*> m_setCellPtr;

public:
	CCellDeserializer(std::shared_ptr<CSheet> pSheet):m_pSheet(pSheet){}
	virtual ~CCellDeserializer(){}

	template<class T>
	void Deserialize(const TCHAR* lpszRootName,T& t,ENABLE_IF_SERIALIZE)
	{
		//Deserialize
		t.serialize(*this);
		m_setCellPtr.clear();
	}

	template<class T>
	void Deserialize(const TCHAR* lpszRootName,T& t,ENABLE_IF_SAVE_LOAD)
	{
		//Deserialize
		t.load(*this);
		m_setCellPtr.clear();
	}

	template<class T>
	void Deserialize(const TCHAR* lpszRootName,T* pT)
	{
		Deserialize(lpszRootName,*pT);
	}
	template<class T>
	void Deserialize(const TCHAR* lpszRootName,std::shared_ptr<T> pT)
	{
		Deserialize(lpszRootName,*pT);
	}

	template<class char_type,class T>
	void operator()(const char_type* lpszName,T& t)
	{
		if(auto spSheet=m_pSheet.lock()){
			if(!spSheet->Empty()){
				std::wstring wstrName(lpszName,(lpszName+strlen(lpszName)));
				auto pCol=spSheet->Index2Pointer<ColTag, AllTag>(0);
				for(auto rowIter=spSheet->RowAllBegin(),rowEnd=spSheet->RowAllEnd();rowIter!=rowEnd;++rowIter){
					auto pCell = CSheet::Cell(rowIter->DataPtr, pCol);
					if(m_setCellPtr.find(pCell.get()) == m_setCellPtr.end() && pCol->Cell(rowIter->DataPtr.get())->GetString()==wstrName){
						m_setCellPtr.insert(pCell.get());
						DeserializeValue(t,rowIter->DataPtr.get(),spSheet->Index2Pointer<ColTag, AllTag>(1).get());
						break;
					}
				}
			}
		}
	}

	template<class char_type, class T, class U>
	void operator()(const char_type* lpszName, T& t, U& u)
	{
		if (auto spSheet = m_pSheet.lock()) {
			if (!spSheet->Empty()) {
				std::wstring wstrName(lpszName, (lpszName + strlen(lpszName)));
				auto pCol = spSheet->Index2Pointer<ColTag, AllTag>(0);
				for (auto rowIter = spSheet->RowAllBegin(), rowEnd = spSheet->RowAllEnd(); rowIter != rowEnd; ++rowIter) {
					auto pCell = CSheet::Cell(rowIter->DataPtr, pCol);
					if (m_setCellPtr.find(pCell.get()) == m_setCellPtr.end() && pCol->Cell(rowIter->DataPtr.get())->GetString() == wstrName) {
						m_setCellPtr.insert(pCell.get());
						DeserializeValue(t, rowIter->DataPtr.get(), spSheet->Index2Pointer<ColTag, AllTag>(1).get());
						break;
					}
				}
			}
		}
	}

	//For base
	template<class T>
	void DeserializeValue(T& t,CRow* pRow,CColumn* pColumn,ENABLE_IF_DEFAULT)
	{
		t=boost::lexical_cast<T>(pColumn->Cell(pRow)->GetString());
	}
	//For std::string boost::lexical_cast couldn't cast std::wstring to std::string
	void DeserializeValue(std::string& t,CRow* pRow,CColumn* pColumn)
	{
		t=wstr2str(pColumn->Cell(pRow)->GetString());
	}
	//For Color
	void DeserializeValue(CColor& t,CRow* pRow,CColumn* pColumn)
	{
		if(auto p=std::dynamic_pointer_cast<CColorCell>(pColumn->Cell(pRow))){
			t=p->GetColor();
		}
	}
	//For Font
	void DeserializeValue(CFont& t,CRow* pRow,CColumn* pColumn)
	{
		if(auto p=std::dynamic_pointer_cast<CFontCell>(pColumn->Cell(pRow))){
			t=p->GetFont();
//			t.Create();
		}
	}	
	//For enum
	template<class T>
	void DeserializeValue(T& t,CRow* pRow,CColumn* pColumn,ENABLE_IF_ENUM)
	{
		t=static_cast<T>(boost::lexical_cast<int>(pColumn->Cell(pRow)->GetString()));
	}
	
	//For serialize
	template<class T>
	void DeserializeValue(T& t,CRow* pRow,CColumn* pColumn,ENABLE_IF_SERIALIZE)
	{
		if(auto pSheet=std::dynamic_pointer_cast<CSheet>(pColumn->Cell(pRow))){
			t.serialize(CCellDeserializer(pSheet));
		}
	}

	//For save load
	template<class T>
	void DeserializeValue(T& t, CRow* pRow, CColumn* pColumn, ENABLE_IF_SAVE_LOAD)
	{
		if (auto pSheet = std::dynamic_pointer_cast<CSheet>(pColumn->Cell(pRow))) {
			t.load(CCellDeserializer(pSheet));
		}
	}

	//For vector
	template<class T>
	void DeserializeValue(std::vector<T>& t,CRow* pRow,CColumn* pColumn)
	{
		//t.clear();
		if(auto spSheet = std::dynamic_pointer_cast<CSheet>(CSheet::Cell(pRow, pColumn))){
			auto& rowAllDict = spSheet->GetDictionary<RowTag, AllTag>();
			auto& rowDict = rowAllDict.get<IndexTag>();
			for(auto rowIter=rowDict.find(0);rowIter!=rowDict.end();rowIter++){
				//T val;
				DeserializeValue(t.at((size_t)std::distance(rowDict.find(0), rowIter)),rowIter->DataPtr.get(),spSheet->Index2Pointer<ColTag, AllTag>(1).get());
				//t.push_back(val);
			}
			
		}
	}
	//For shared_ptr
	template<class T>
	void DeserializeValue(std::shared_ptr<T>& t,CRow* pRow,CColumn* pColumn)
	{
		DeserializeValue(*t,pRow,pColumn);
	}
};

