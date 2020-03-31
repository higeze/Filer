#pragma once
#include "SheetCell.h"
#include "Row.h"
#include "PropertySerializer.h"
#include "SheetEventArgs.h"
#include "CellProperty.h"
#include "MyMPL.h"
#include <memory>

template<typename T>
class CPropertySheetCell:public CSheetCell
{
public:
	virtual bool CanResizeRow()const override{return true;}
	virtual bool CanResizeColumn()const override{return false;}
	virtual bool HasSheetCell()override { return true; }
	virtual bool IsVirtualPage()override { return true; }

	//TODO Not size but index

	void Resize(int row, int col)override
	{
		int curRowSize = GetContainer<RowTag, AllTag>().size() - 1;
		int curColSize = GetContainer<ColTag, AllTag>().size() - 1;

		if(row==curRowSize)return;

		if(CanResizeRow() && row>curRowSize){
			for(auto i=0;i<row-curRowSize;i++){
				auto spRow=std::make_shared<CRow>(this);
				PushRow(spRow);
				auto pColValue= Index2Pointer<ColTag, AllTag>(1);

				CCellSerializer serializer(std::dynamic_pointer_cast<CSheet>(Cell(m_pRow,m_pColumn)),GetSheetProperty(), CCell::GetCellPropertyPtr());
				serializer.SerializeValue(CreateInstance<T>(),spRow.get(),pColValue.get());
			}
		}else if(CanResizeRow() && row<curRowSize && row>0){
			for(auto i=0;i<curRowSize-row;i++){
				EraseRow(m_allRows.back());
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
		std::shared_ptr<SheetProperty> spSheetProperty = nullptr,
		std::shared_ptr<CellProperty> spCellProperty = nullptr,
		CMenu* pMenu=nullptr)
		:CSheetCell(pSheet,pRow,pColumn,spSheetProperty, spCellProperty){}
	virtual ~CPropertySheetCell(){}
};
