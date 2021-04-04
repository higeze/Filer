#include "ApplicationProperty.h"
#include "FilerProperty.h"

//#include "resource.h"
//std::shared_ptr<CApplicationProperty> g_spApplicationProperty;
void SerializeProperty(CFilerWnd* pProp);
//HWND g_main;

#include <cereal/cereal.hpp>
/* Following fix is applied*/
/* https://github.com/USCiLab/cereal/commit/a5a30953125e70b115a28dd76b64adf3c97cc883#diff-92ab9a36df5d8e9f7076f2fdec59492d1ac2d9cf27ea046767a7fc4d542ef3dcR7 */

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

#include <fstream>

//namespace cereal
//{
//	template<class Archive>
//	void save(Archive& archive,
//			  std::wstring const& value,
//		std::uint32_t const version )
//	{
//		std::string str = "test";//wstr2str(value);
//		//archive(CEREAL_NVP(str));
//		archive(str);
//	}
//
//	template<class Archive>
//	void load(Archive& archive,
//			  std::wstring& value,
//		std::uint32_t const version )
//	{
//		std::string str;
//		//archive(CEREAL_NVP(str)); 
//		archive(str); 
//		value = str2wstr(str);
//	}
//
//	template <class Archive> 
//    struct specialize<Archive, std::wstring, cereal::specialization::non_member_load_save> {};
//}


//namespace cereal
//{
	//template<class A>
	//std::string CEREAL_SAVE_MINIMAL_FUNCTION_NAME( A const &, const std::wstring &in )
	//{
	//	return wstr2str( in );
	//}

	//template<class A>
	//void CEREAL_LOAD_MINIMAL_FUNCTION_NAME( A const &, std::wstring &out, const std::string &in )
	//{
	//	out = str2wstr( in );
	//}
//}
//
//CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( std::wstring, cereal::specialization::non_member_load_save_minimal );

//template <class Archive>
//double save_minimal( 
//  Archive const &,
//  std::wstring const & md )
//{
//  return wstr2str(md);
//}
//
//template <class Archive>
//void load_minimal( Archive const &,
//  std::wstring & md,
//  std::string const & value )
//{
//  //md.d = value;
//}
//
//CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( std::wstring, cereal::specialization::non_member_load_save_minimal );

//#include <boost/locale.hpp>

//namespace cereal
//{
//	template<class A>
//	std::string CEREAL_SAVE_MINIMAL_FUNCTION_NAME( A const &, const std::wstring &in )
//	{
//		auto str = wstr2str(in);
//		return str;
//	}
//
//	template<class A>
//	void CEREAL_LOAD_MINIMAL_FUNCTION_NAME( A const &, std::wstring &out, const std::string &in )
//	{
//		//out = boost::locale::conv::utf_to_utf<wchar_t>( in );
//	}
//}
//
//CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( std::wstring, cereal::specialization::non_member_load_save_minimal );

//#define CEREAL_NVP_W(T) ::cereal::make_nvp_w(#T, T)
//namespace cereal
//{
//  template <class T> inline
//  NameValuePair<T> make_nvp_w( const char * name, T && value )
//  {
//    return {name, std::forward<T>(value)};
//  }
//}

struct CChildBase
{
	std::wstring Name;
	std::vector<int> IDs;

	CChildBase() = default;
	virtual ~CChildBase() = default;
	CChildBase(std::wstring name, std::vector<int> ids)
	:Name(name), IDs(ids){}

	template<typename Archive>
	void save(Archive& ar) const
	{
		std::string temp = wstr2str(Name);
		ar(cereal::make_nvp("Name", temp), CEREAL_NVP(IDs));
//		ar(CEREAL_NVP(Name), CEREAL_NVP(IDs));
	}

	template<typename Archive>
	void load(Archive& ar)
	{
		std::string temp;
		ar(cereal::make_nvp("Name", temp), CEREAL_NVP(IDs));
		Name = str2wstr(temp);
//		ar(CEREAL_NVP(Name), CEREAL_NVP(IDs));
	}
};

struct CChildDerived :public CChildBase
{
	std::wstring Section;

	CChildDerived() = default;
	virtual ~CChildDerived() = default;
	CChildDerived(std::wstring name, std::vector<int> ids, std::wstring section)
		:CChildBase(name, ids), Section(section){}


	template<typename Archive>
	void save(Archive& ar) const
	{
		ar(cereal::base_class<CChildBase>(this));
		std::string temp = wstr2str(Section);
		ar(cereal::make_nvp("Section", temp));
	}

	template<typename Archive>
	void load(Archive& ar)
	{
		ar(cereal::base_class<CChildBase>(this));
		std::string temp;
		ar(cereal::make_nvp("Section", temp));
		Section = str2wstr(temp);
	}
};

struct CChildDerived2 :public CChildBase
{
	std::wstring Section;
	CChildDerived2(std::wstring name = std::wstring(), std::vector<int> ids = std::vector<int>(), std::wstring section = std::wstring())
		:CChildBase(name, ids), Section(L"Derived222222"){}
	virtual ~CChildDerived2() = default;

	template<typename Archive>
	void save(Archive& ar) const
	{
		ar(cereal::base_class<CChildBase>(this));
		std::string temp = wstr2str(Section);
		ar(cereal::make_nvp("Section", temp));
	}

	template<typename Archive>
	void load(Archive& ar)
	{
		ar(cereal::base_class<CChildBase>(this));
		std::string temp;
		ar(cereal::make_nvp("Section", temp));
		Section = str2wstr(temp);
	}
};

struct CParent
{
	std::vector<std::shared_ptr<CChildBase>> Children;

	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(Children));
	}
};



CEREAL_REGISTER_TYPE(CChildDerived)
CEREAL_REGISTER_POLYMORPHIC_RELATION(CChildBase, CChildDerived)
CEREAL_REGISTER_TYPE(CChildDerived2)
CEREAL_REGISTER_POLYMORPHIC_RELATION(CChildBase, CChildDerived2)

// // A pure virtual base class
//struct BaseClass
//{
//  virtual void sayType() = 0;
//};
//
//// A class derived from BaseClass
//struct DerivedClassOne : public BaseClass
//{
//  void sayType();
//
//  int x;
//
//  template<class Archive>
//  void serialize( Archive & ar )
//  { ar( x ); }
//};
//
//// Another class derived from BaseClass
//struct EmbarrassingDerivedClass : public BaseClass
//{
//  void sayType();
//
//  float y;
//
//  template<class Archive>
//  void serialize( Archive & ar )
//  { ar( y ); }
//};
//
//// Include any archives you plan on using with your type before you register it
//// Note that this could be done in any other location so long as it was prior
//// to this file being included
//#include <cereal/archives/binary.hpp>
//#include <cereal/archives/xml.hpp>
//#include <cereal/archives/json.hpp>
//
//// Register DerivedClassOne
//CEREAL_REGISTER_TYPE(DerivedClassOne);
//
//// Register EmbarassingDerivedClass with a less embarrasing name
//CEREAL_REGISTER_TYPE_WITH_NAME(EmbarrassingDerivedClass, "DerivedClassTwo");
//
//// Note that there is no need to register the base class, only derived classes
////  However, since we did not use cereal::base_class, we need to clarify
////  the relationship (more on this later)
//CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseClass, DerivedClassOne)
//CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseClass, EmbarrassingDerivedClass)


