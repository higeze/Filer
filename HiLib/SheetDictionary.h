#pragma once
class CRow;
#include "Column.h"
#include "MyFriendSerializer.h"

struct Data
{
	typedef int size_type;
	typedef size_type index_type;
	size_type Index;
	Data(size_type dispIndex = 0)
		: Index(dispIndex){}
	virtual ~Data(){}

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Index", Index);
	}
};

struct RowData:public Data
{
	typedef CRow* pointer_type;
	std::shared_ptr<CRow> DataPtr;
	RowData(size_type dispIndex, std::shared_ptr<CRow> rowPtr)
		: Data(dispIndex), DataPtr(rowPtr) {}
	virtual ~RowData(){}
	const CRow* Get()const{return DataPtr.get();}

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		Data::serialize(ar);
		ar("Data", DataPtr);
	}
};

struct ColumnData:public Data
{
	typedef CColumn* pointer_type;
	std::shared_ptr<CColumn> DataPtr;
	ColumnData(size_type dispIndex = 0, std::shared_ptr<CColumn> columnPtr = nullptr)
		: Data(dispIndex), DataPtr(columnPtr) {}
	virtual ~ColumnData(){}
	const CColumn* Get()const{return DataPtr.get();}

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		Data::serialize(ar);
		ar("Data", DataPtr);
	}
};


struct IndexTag {};
struct PointerTag {};

typedef boost::multi_index::multi_index_container<
	RowData,
	boost::multi_index::indexed_by<
		boost::multi_index::ordered_unique<boost::multi_index::tag<IndexTag>, boost::multi_index::member<Data, Data::size_type, &Data::Index>>,
		boost::multi_index::hashed_unique<boost::multi_index::tag<PointerTag>, boost::multi_index::const_mem_fun<RowData, const CRow*, &RowData::Get>>
	>
> RowDictionary;

typedef boost::multi_index::multi_index_container<
	ColumnData,
	boost::multi_index::indexed_by<
		boost::multi_index::ordered_unique<boost::multi_index::tag<IndexTag>, boost::multi_index::member<Data, Data::size_type, &Data::Index>>,
		boost::multi_index::hashed_unique<boost::multi_index::tag<PointerTag>, boost::multi_index::const_mem_fun<ColumnData, const CColumn*, &ColumnData::Get>>
	>
> ColumnDictionary;
