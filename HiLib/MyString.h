#pragma once
#include <string>
#include <vector>
#include <Windows.h>
typedef std::basic_string<TCHAR> tstring;

#define REGEX_FILEEXT	L"\\.[^.\\\\\\/:*?\"<>|\\r\\n]+$"
#define REGEX_FILENAME_A	"[^\\\\\\/:*?\\\"<>|\\r\\n]+$"
#define REGEX_FILENAME_W	L"[^\\\\\\/:*?\\\"<>|\\r\\n]+$"

/****************/
/* tstring_cast */
/****************/
template<typename TRect>
inline const TRect* tstring_cast(const char* str,const wchar_t* wstr)
{
	return str;
}
template<>
inline const wchar_t* tstring_cast<wchar_t>(const char* str,const wchar_t* wstr)
{
	return wstr;
}

//template<class char_type,const char* pC,const wchar_t* pW>
//struct tstring_cast {
//	static const char_type value = pC;
//};
//
//template<const char* pC,const wchar_t* pW>
//struct tstring_cast<wchar_t,pC,pW> {
//	static const wchar_t value = pW;
//};


#define TSTRING_CAST(char_type,str) tstring_cast<char_type>(str,L##str)
/****************/
/* tchar_cast */
/****************/
/*
template<typename T>
inline const T tchar_cast(char ch,wchar_t wch)
{
	return ch;
}
template<>
inline const wchar_t tchar_cast<wchar_t>(char ch,wchar_t wch)
{
	return wch;
}
*/
template<class char_type,char c,wchar_t w>
struct tchar_cast {
	static const char_type value = c;
};

template<char c,wchar_t w>
struct tchar_cast<wchar_t,c,w> {
	static const wchar_t value = w;
};

#define	TCHAR_CAST(char_type,c) tchar_cast<char_type,c,L##c>::value

/*************/
/* GetBuffer */
/*************/

template<typename char_type>
char_type* GetBuffer(std::basic_string<char_type>& str,int n)
{
	str.resize(n+1);
	return &str[0];
}

/*****************/
/* ReleaseBuffer */
/*****************/
template<typename char_type>
void ReleaseBuffer(std::basic_string<char_type>& str)
{
	typename std::basic_string<char_type>::size_type size=str.find_first_of(TCHAR_CAST(char_type,'\0'));
	str.resize(size);
}

/*********/
/* IsNaN */
/*********/
template<typename char_type>
bool IsNaN(const std::basic_string<char_type>& str)
{
	size_t i=0;
	if(str[0]==TCHAR_CAST(char_type,'-')){
		i=1;
	}else{
		i=0;
	}
	
	for(size_t iLen=str.length();i<iLen;i++){
		if(!(::isdigit(str[i])||
			str[i]==TCHAR_CAST(char_type,'.')||
			str[i]==TCHAR_CAST(char_type,'e')||
			str[i]==TCHAR_CAST(char_type,'E')||
			str[i]==TCHAR_CAST(char_type,'+')||
			str[i]==TCHAR_CAST(char_type,'-'))){
				return true;
		}
	}
	return false;
}

std::wstring str2wstr(const std::string& str);
//std::wstring str2wstr(const std::wstring& str){return str;}

//std::string wstr2str(const std::string& str){return str;}
std::string wstr2str(const std::wstring& wstr);


template<typename TRect>
std::basic_string<TRect> RoundDouble2String(const double& arg, int precision)
{
	std::basic_string<TRect> result;
	std::basic_stringstream<TRect> sstream;
	sstream.setf(std::ios::fixed);
	sstream.precision(precision);
	if(!(sstream<<arg && sstream>>result)){
		throw std::exception("Error on RoundDouble2String");
	}	
	int dotIndex=result.find_first_of(TCHAR_CAST(TRect,'.'));
	if(dotIndex!=std::basic_string<TRect>::npos){
		int nonZeroIndex=dotIndex-1;
		for(unsigned i=dotIndex+1;i<result.size();i++){
			if(i == dotIndex+1 || result[i]!=TCHAR_CAST(TRect,'0')){
				nonZeroIndex=i;
			}		
		}
		result.erase(nonZeroIndex+1);
	}
	return result;
}

template<typename TRect,typename U>
std::basic_string<TRect> roundtostring(const U& arg, int precision)
{
	std::basic_string<TRect> result;
	std::basic_stringstream<TRect> sstream;
	sstream.setf(std::ios::fixed);
	sstream.precision(precision);
	if(!(sstream<<arg && sstream>>result))throw std::exception("Error on roundtostring");
	int dotIndex=result.find_first_of(L'.');
	if(dotIndex!=std::basic_string<TRect>::npos){
		int nonZeroIndex=dotIndex-1;
		for(unsigned i=dotIndex+1;i<result.size();i++){
			if(result[i]!=TCHAR_CAST(TRect,'0')){
				nonZeroIndex=i;
			}		
		}
		result.erase(nonZeroIndex+1);
	}
	return result;
}



namespace my
{
	template<typename TRect>
	bool find(const std::basic_string<TRect>& str1, const std::basic_string<TRect>& str2, bool matchCase, bool matchWholeWord)
	{
		bool find = false;
		if(matchCase && matchWholeWord){
			find = str1 == str2;
		}else if(!matchCase && matchWholeWord){
			find = boost::iequals(str1, str2);
		}else if(matchCase && !matchWholeWord){
			find = str1.find(str2) == std::wstring::npos;
		}else{
			find = !boost::algorithm::ifind_first(str1, str2).empty();
		}
		return find;
	}
}

template<typename _Char>
struct find_insensitive
{
	using string_type = std::basic_string<_Char, std::char_traits<_Char>, std::allocator<_Char>>;
private:
	string_type m_needle;
	size_t m_position;
public:
	find_insensitive(const string_type& needle, const size_t& position)
		:m_needle(needle), m_position(position) {}

	template<typename _Char>
	friend auto operator | (const string_type& haystack, const find_insensitive<_Char>& arguments)->size_t
	{
		if (haystack.empty() || arguments.m_needle.empty()) {
			return string_type::npos;
		}
		auto iter = std::search(
			haystack.cbegin() + arguments.m_position, haystack.cend(),
			arguments.m_needle.cbegin(), arguments.m_needle.cend(),
			[](const _Char& ch1, const _Char& ch2) { return std::toupper(ch1) == std::toupper(ch2); }
		);

		if (iter == haystack.cend()) {
			return string_type::npos;
		}else {
			return std::distance(haystack.cbegin(), iter);
		}
	}

};