#pragma once
#include <string>
#include "encoding_type.h"

class CTextEnDecoder
{
private:
	std::string bom_utf8 = {(char)0xEF, (char)0xBB, (char)0xBF};
	std::string bom_utf16le = { (char)0xFF, (char)0xFE };
	std::string bom_utf16be = { (char)0xFE, (char)0xFF };

	std::unordered_map<encoding_type,
		std::pair<
		std::function<std::wstring(const std::string&)>,
		std::function<std::string(const std::wstring&)>
		>> m_map;
public:
	CTextEnDecoder();
	static CTextEnDecoder* GetInstance()
	{
		static CTextEnDecoder that;
		return &that;
	}	
	encoding_type DetectEncoding(const std::string& str);
	std::wstring Decode(const std::string& str, const encoding_type& enc);
	std::string Encode(const std::wstring& str, const encoding_type& enc);
private:
    encoding_type DetectEncodingByBOM(const std::string& str);
	encoding_type DetectEncodingByParse(const std::string& str);
	encoding_type DetectEncodingByMultiLanguage(const std::string& str);
};
