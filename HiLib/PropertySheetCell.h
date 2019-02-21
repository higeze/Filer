#pragma once
#include "SheetCell.h"
#include "Row.h"
#include "PropertySerializer.h"
#include "SheetEventArgs.h"
#include "CellProperty.h"

template<typename T>
class CPropertySheetCell:public CSheetCell
{
public:
	virtual bool CanResizeRow()const override{return true;}
	virtual bool CanResizeColumn()const override{return false;}

	//TODO Not size but index
	void Resize(int row, int col)override
	{
		int curRowSize = GetMaxIndex<RowTag, AllTag>();
		int curColSize = GetMaxIndex<ColTag, AllTag>();

		if(row==curRowSize)return;

		if(CanResizeRow() && row>curRowSize){
			for(auto i=0;i<row-curRowSize;i++){
				auto spRow=std::make_shared<CChildRow>(this);
				InsertRow(100,spRow);
				auto pColValue= Index2Pointer<ColTag, AllTag>(1);

				CCellSerializer serializer(std::dynamic_pointer_cast<CSheet>(Cell(m_pRow,m_pColumn)),m_spHeaderProperty,m_spFilterProperty,m_spCellProperty);
				serializer.SerializeValue(std::make_shared<T>(),spRow.get(),pColValue.get());
			}
		}else if(CanResizeRow() && row<curRowSize && row>0){
			for(auto i=0;i<curRowSize-row;i++){
				auto& indexDic = m_rowAllDictionary.get<IndexTag>();
				auto iter=boost::prior(indexDic.end());
				EraseRow(iter->DataPtr.get());
			}
		}
		OnPropertyChanged(L"value");
		SubmitUpdate();

		//Column is not resizable
	}
public:

	//Constructor
	CPropertySheetCell(
		CSheet* pSheet = nullptr,
		CRow* pRow = nullptr,
		CColumn* pColumn = nullptr,
		std::shared_ptr<CellProperty> spProperty = nullptr,
		std::shared_ptr<HeaderProperty> spHeaderProperty = nullptr,
		std::shared_ptr<CellProperty> spFilterProperty = nullptr,
		std::shared_ptr<CellProperty> spCellProperty = nullptr,
		CMenu* pMenu=nullptr)
		:CSheetCell(pSheet,pRow,pColumn,spProperty,spHeaderProperty,spFilterProperty,spCellProperty){}
	virtual ~CPropertySheetCell(){}
};
