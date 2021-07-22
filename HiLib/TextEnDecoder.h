#pragma once
#include <string>
#include "encoding_type.h"

class CTextEnDecoder
{
private:
	std::vector<byte> bom_non = {};
	std::vector<byte> bom_utf8 = {0xEF, 0xBB, 0xBF};
	std::vector<byte> bom_utf16le = { 0xFF, 0xFE };
	std::vector<byte> bom_utf16be = { 0xFE, 0xFF };

	std::unordered_map<encoding_type,
		std::pair<
		std::function<std::wstring(const std::vector<byte>&)>,
		std::function<std::vector<byte>(const std::wstring&)>
		>> m_map;

	std::vector<byte> wstring_to_bytes(const std::wstring& wstr);
	std::vector<byte> string_to_bytes(const std::string& str);
	std::wstring bytes_to_wstring(const std::vector<byte>& bytes, const size_t offset);
	std::string bytes_to_string(const std::vector<byte>& bytes, const size_t offset);

public:
	CTextEnDecoder();
	static CTextEnDecoder* GetInstance()
	{
		static CTextEnDecoder that;
		return &that;
	}	
	encoding_type DetectEncoding(const std::vector<byte>& str);

	std::wstring Decode(const std::vector<byte>& str, const encoding_type& enc);
	std::vector<byte> Encode(const std::wstring& str, const encoding_type& enc);
private:
    encoding_type DetectEncodingByBOM(const std::vector<byte>& str);
	encoding_type DetectEncodingByParse(const std::vector<byte>& str);
	encoding_type DetectEncodingByMultiLanguage(const std::vector<byte>& str);
};
