#pragma once

#include "MyMPL.h"
#include "MyDC.h"
#include "MyString.h"
#include "PropertyGridView.h"

#include "Row.h"
#include "RowIndexColumn.h"
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
#include "CellProperty.h"
#include "ReactiveProperty.h"

#include "BindTextCell.h"

class CCellSerializer
{
private:
	CSheet* m_pSheet;
public:
	CCellSerializer(CSheet* pSheet)
		:m_pSheet(pSheet){}
	virtual ~CCellSerializer() = default;

	template<class T, typename ENABLE_IF_SERIALIZE>
	void Serialize(std::shared_ptr<CPropertyGridView> pGrid,const TCHAR* lpszRootName,T& t)
	{
		//Serialize
		t.serialize(*this);
	}
	template<class T, typename ENABLE_IF_SAVE_LOAD>
	void Serialize(std::shared_ptr<CPropertyGridView> pGrid,const TCHAR* lpszRootName,T& t)
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
		std::shared_ptr<CRow> pItemRow;
		std::shared_ptr<CColumn> pColName;
		std::shared_ptr<CColumn> pColValue;	

		if(m_pSheet->Empty()){
			m_pSheet->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(m_pSheet));
			pItemRow = std::make_shared<CRow>(m_pSheet);
			m_pSheet->PushRows(
				m_pSheet->GetNameHeaderRowPtr(),
				pItemRow);
			m_pSheet->SetFrozenCount<RowTag>(1);

			pColName = std::make_shared<CPropertyNameColumn>(m_pSheet);
			pColValue = std::make_shared<CPropertyValueColumn>(m_pSheet);
			m_pSheet->PushColumns(
				pColName,
				pColValue);
			m_pSheet->SetFrozenCount<ColTag>(1);
		}else{
			pItemRow=std::make_shared<CRow>(m_pSheet);
			m_pSheet->PushRow(pItemRow);
			pColName = m_pSheet->Index2Pointer<ColTag, AllTag>(0);
			pColValue = m_pSheet->Index2Pointer<ColTag, AllTag>(1);
		}

		CSheet::Cell(pItemRow, pColName)->SetStringCore(std::wstring(lpszName,(lpszName+strlen(lpszName))));
		SerializeValue(t,pItemRow.get(),pColValue.get());	
	}

	//For base
	template<class T, typename ENABLE_IF_DEFAULT>
	void SerializeValue(T& t,CRow* pRow,CColumn* pCol)
	{
		pCol->Cell(pRow) = //->SetStringCore(boost::lexical_cast<std::wstring>(t));
			std::make_shared<CBindTextCell2<T>>(
				m_pSheet, pRow, pCol, m_pSheet->GetCellProperty(),
				[&t]() {return boost::lexical_cast<std::wstring>(t); },
				[&t](const std::wstring& str) {t = boost::lexical_cast<T>(str); },
				arg<"editmode"_s>() = EditMode::LButtonDownEdit);

	}

	//For std::string boost::lexical cast couldn't cast std::string to std::wstring
	void SerializeValue(std::string& t,CRow* pRow,CColumn* pCol)
	{
		pCol->Cell(pRow) = //->SetStringCore(str2wstr(t));
			std::make_shared<CBindTextCell2<std::string>>(
				m_pSheet, pRow, pCol, m_pSheet->GetCellProperty(),
				[&t]() {return str2wstr(t); },
				[&t](const std::wstring& str) {t = wstr2str(str); },
				arg<"editmode"_s>() = EditMode::LButtonDownEdit);

	}

	//For Color
	void SerializeValue(CColorF& t,CRow* pRow,CColumn* pCol)
	{
		pCol->Cell(pRow)=std::make_shared<CColorCell>(
			m_pSheet,
			pRow,
			pCol,
			m_pSheet->GetCellProperty(),
			t);
	}
	//For Font
	void SerializeValue(CFontF& t,CRow* pRow,CColumn* pCol)
	{
		pCol->Cell(pRow)=std::make_shared<CFontCell>(
			m_pSheet,
			pRow,
			pCol,
			m_pSheet->GetCellProperty(),
			t);
	}

	//For bool
	void SerializeValue(bool& t,CRow* pRow,CColumn* pCol)
	{
		pCol->Cell(pRow)=std::make_shared<CBoolCell>(
			m_pSheet,
			pRow,
			pCol,
			m_pSheet->GetCellProperty(),
			t);
	}

	//For enum
	template<class T, typename ENABLE_IF_ENUM>
	void SerializeValue(T& t,CRow* pRow,CColumn* pCol)
	{
		pCol->Cell(pRow)->SetStringCore(boost::lexical_cast<std::wstring>(static_cast<int>(t)));
	}
	
	//For serialize
	template<class T, typename ENABLE_IF_SERIALIZE>
	void SerializeValue(T& t,CRow* pRow,CColumn* pCol)
	{
		std::shared_ptr<CSheetCell> spSheetCell(
			std::make_shared<CSheetCell>(
			m_pSheet,
			pRow,
			pCol,
			m_pSheet->GetSheetProperty(),
			m_pSheet->GetCellProperty()));

		pCol->Cell(pRow)=spSheetCell;

		t.serialize(CCellSerializer(spSheetCell.get()));
		spSheetCell->UpdateAll();
	}

	//For save load
	template<class T, typename ENABLE_IF_SAVE_LOAD>
	void SerializeValue(T& t, CRow* pRow, CColumn* pCol)
	{
		std::shared_ptr<CSheetCell> spSheetCell(
			std::make_shared<CSheetCell>(
				m_pSheet,
				pRow,
				pCol,
				m_pSheet->GetSheetProperty(),
				m_pSheet->GetCellProperty()));

		pCol->Cell(pRow) = spSheetCell;

		t.save(CCellSerializer(spSheetCell.get()));
		spSheetCell->UpdateAll();
	}

	//For ptr
	template<class T, typename ENABLE_IF_PTR>
	void SerializeValue(T& t, CRow* pRow, CColumn* pCol)
	{
		SerializeValue(*t, pRow, pCol);
	}

	//For vector
	template<class T>
	void SerializeValue(std::vector<T>& t, CRow* pRow, CColumn* pCol)
	{
		auto a = 2;
	}

	//For observable_tuple_vector
	template<class T>
	void SerializeValue(ReactiveProperty<std::tuple<T>>& t, CRow* pRow, CColumn* pCol)
	{
		auto spCell = std::make_shared<CBindPropertySheetCell<T>>(
			m_pSheet,
			pRow,
			pCol,
			m_pSheet->GetSheetProperty(),
			m_pSheet->GetCellProperty(),
			t);
		CSheet::Cell(pRow, pCol) = spCell;
		spCell->UpdateAll();
	}
	//For shared_ptr
	template<class T>
	void SerializeValue(std::shared_ptr<T>& t,CRow* pRow,CColumn* pCol)
	{
		SerializeValue(*t,pRow,pCol);
	}
	//For unique_ptr
	template<class T>
	void SerializeValue(std::unique_ptr<T>& t, CRow* pRow, CColumn* pCol)
	{
		SerializeValue(*t, pRow, pCol);
	}
	//For single tuple
	template<class T>
	void SerializeValue(std::tuple<T>& t, CRow* pRow, CColumn* pCol)
	{
		SerializeValue(std::get<T>(t), pRow, pCol);
	}

};


class CCellDeserializer
{
private:
	CSheet* m_pSheet;
//	std::unordered_set<CCell*> m_setCellPtr;

public:
	CCellDeserializer(CSheet* pSheet)
		:m_pSheet(pSheet){}
	virtual ~CCellDeserializer() = default;

	template<class T, typename ENABLE_IF_SERIALIZE>
	void Deserialize(const TCHAR* lpszRootName,T& t)
	{
		//Deserialize
		t.serialize(*this);
//		m_setCellPtr.clear();
	}

	template<class T, typename ENABLE_IF_SAVE_LOAD>
	void Deserialize(const TCHAR* lpszRootName,T& t)
	{
		//Deserialize
		t.load(*this);
//		m_setCellPtr.clear();
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

	//template<class char_type,class T>
	//void operator()(const char_type* lpszName,T& t)
	//{
	//	if(auto spSheet=m_pSheet.lock()){
	//		if(!spSheet->Empty()){
	//			std::wstring wstrName(lpszName,(lpszName+strlen(lpszName)));
	//			auto pCol=spSheet->Index2Pointer<ColTag, AllTag>(0);
	//			for(auto rowIter=spSheet->RowAllBegin(),rowEnd=spSheet->RowAllEnd();rowIter!=rowEnd;++rowIter){
	//				auto pCell = CSheet::Cell(rowIter->DataPtr, pCol);
	//				if(m_setCellPtr.find(pCell.get()) == m_setCellPtr.end() && pCol->Cell(rowIter->DataPtr.get())->GetString()==wstrName){
	//					m_setCellPtr.insert(pCell.get());
	//					DeserializeValue(t,rowIter->DataPtr.get(),spSheet->Index2Pointer<ColTag, AllTag>(1).get());
	//					break;
	//				}
	//			}
	//		}
	//	}
	//}

	template<class char_type, class T, class... U>
	void operator()(const char_type* lpszName, T& t, U... args)
	{
		if (!m_pSheet->Empty()) {
			std::wstring wstrName(lpszName, (lpszName + strlen(lpszName)));
			auto spNameCol = m_pSheet->Index2Pointer<ColTag, AllTag>(0);
			auto spValueCol = m_pSheet->Index2Pointer<ColTag, AllTag>(1);
			for (auto rowIter = m_pSheet->GetContainer<RowTag, AllTag>().begin() + m_pSheet->GetFrozenCount<RowTag>(), rowEnd = m_pSheet->GetContainer<RowTag, AllTag>().end(); rowIter != rowEnd; ++rowIter) {
				//auto pCell = CSheet::Cell(*rowIter, pCol);
				if (/*m_setCellPtr.find(pCell.get()) == m_setCellPtr.end() && */CSheet::Cell(*rowIter, spNameCol)->GetString() == wstrName) {
					//m_setCellPtr.insert(pCell.get());
					DeserializeValue(t, rowIter->get(), spValueCol.get());
					break;
				}
			}
		}
	}

	//For base
	template<class T, typename ENABLE_IF_DEFAULT>
	void DeserializeValue(T& t,CRow* pRow,CColumn* pColumn)
	{
		t=boost::lexical_cast<T>(pColumn->Cell(pRow)->GetString());
	}

	//For std::string boost::lexical_cast couldn't cast std::wstring to std::string
	void DeserializeValue(std::string& t,CRow* pRow,CColumn* pColumn)
	{
		t=wstr2str(pColumn->Cell(pRow)->GetString());
	}

	//For Color
	void DeserializeValue(CColorF& t, CRow* pRow, CColumn* pColumn)
	{
		if(auto p=std::dynamic_pointer_cast<CColorCell>(CSheet::Cell(pRow, pColumn))){
			t=p->GetColor();
		}
	}

	//For Font
	void DeserializeValue(CFontF& t,CRow* pRow,CColumn* pColumn)
	{
		if(auto p=std::dynamic_pointer_cast<CFontCell>(pColumn->Cell(pRow))){
			t=p->GetFont();
		}
	}	
	//For enum
	template<class T, typename ENABLE_IF_ENUM>
	void DeserializeValue(T& t,CRow* pRow,CColumn* pColumn)
	{
		t=static_cast<T>(boost::lexical_cast<int>(pColumn->Cell(pRow)->GetString()));
	}
	
	//For serialize
	template<class T, ENABLE_IF_SERIALIZE>
	void DeserializeValue(T& t,CRow* pRow,CColumn* pColumn)
	{
		if(auto pSheet=std::dynamic_pointer_cast<CSheet>(pColumn->Cell(pRow))){
			t.serialize(CCellDeserializer(pSheet.get()));
		}
	}

	//For save load
	template<class T, ENABLE_IF_SAVE_LOAD>
	void DeserializeValue(T& t, CRow* pRow, CColumn* pColumn)
	{
		if (auto pSheet = std::dynamic_pointer_cast<CSheet>(pColumn->Cell(pRow))) {
			t.load(CCellDeserializer(pSheet.get()));
		}
	}

	//For vector
	template<class T>
	void DeserializeValue(std::vector<T>& t,CRow* pRow,CColumn* pColumn)
	{
		auto& allRows = m_pSheet->GetContainer<RowTag, AllTag>();
		t.clear();
		for(auto rowIter=allRows.begin() + m_pSheet->GetFrozenCount<RowTag>();rowIter!=allRows.end();rowIter++){
			auto val = CreateInstance<T>();
			DeserializeValue(val,rowIter->get(), m_pSheet->Index2Pointer<ColTag, AllTag>(m_pSheet->GetFrozenCount<ColTag>()).get());
			t.push_back(val);
		}
	}

	//For observable_tuple_vector
	template<class T>
	void DeserializeValue(ReactiveProperty<std::tuple<T>>& t, CRow* pRow, CColumn* pColumn)
	{
		//auto& allRows = m_pSheet->GetContainer<RowTag, AllTag>();
		//t.clear();
		//for (auto rowIter = allRows.begin() + m_pSheet->GetFrozenCount<RowTag>(); rowIter != allRows.end(); rowIter++) {
		//	auto val = CreateInstance<T>();
		//	DeserializeValue(val, rowIter->get(), m_pSheet->Index2Pointer<ColTag, AllTag>(m_pSheet->GetFrozenCount<ColTag>()).get());
		//	t.push_back(val);
		//}

		//if (auto spSheet = std::dynamic_pointer_cast<CSheet>(CSheet::Cell(pRow, pColumn))) {
		//	auto& allRows = spSheet->GetContainer<RowTag, AllTag>();
		//	t.clear();
		//	for (auto rowIter = allRows.begin() + spSheet->GetFrozenCount<RowTag>(); rowIter != allRows.end(); rowIter++) {
		//		auto val = CreateInstance<T>();
		//		DeserializeValue(val, rowIter->get(), spSheet->Index2Pointer<ColTag, AllTag>(spSheet->GetFrozenCount<ColTag>()).get());
		//		t.push_back(std::make_tuple(val));
		//	}
		//}

		if (auto spSheet = std::dynamic_pointer_cast<CBindPropertySheetCell<T>>(CSheet::Cell(pRow, pColumn))) {
			auto& itemsSource = spSheet->GetItemsSource();
			t.clear();
			for (auto item : itemsSource) {
				t.push_back(item);
			}
		}
	}
	//For shared_ptr
	template<class T>
	void DeserializeValue(std::shared_ptr<T>& t,CRow* pRow,CColumn* pColumn)
	{
		DeserializeValue(*t,pRow,pColumn);
	}

	//For unique_ptr
	template<class T>
	void DeserializeValue(std::unique_ptr<T>& t, CRow* pRow, CColumn* pColumn)
	{
		DeserializeValue(*t, pRow, pColumn);
	}

	//For single tuple
	template<class T>
	void DeserializeValue(std::tuple<T>& t, CRow* pRow, CColumn* pColumn)
	{
		DeserializeValue(std::get<T>(t), pRow, pColumn);
	}

};

