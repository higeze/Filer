#include "TextEnDecoder.h"
#include "Debug.h"
#include "strconv.h"
#include <MLang.h>

std::vector<byte> CTextEnDecoder::wstring_to_bytes(const std::wstring& wstr)
{
    return std::vector<byte>((byte*)wstr.data(), ((byte*)wstr.data()) + wstr.size() * sizeof(wchar_t));
}
std::vector<byte> CTextEnDecoder::string_to_bytes(const std::string& str)
{
    return std::vector<byte>((byte*)str.data(), ((byte*)str.data()) + str.size() * sizeof(char));
}
std::wstring CTextEnDecoder::bytes_to_wstring(const std::vector<byte>& bytes, const size_t offset)
{
    return std::wstring((wchar_t*)(bytes.data() + offset), (bytes.size() - offset) / sizeof(wchar_t));
}
std::string CTextEnDecoder::bytes_to_string(const std::vector<byte>& bytes, const size_t offset)
{
    return std::string((char*)(bytes.data() + offset), (bytes.size() - offset) / sizeof(char));
}




CTextEnDecoder::CTextEnDecoder()
    :m_map(
        {
            {encoding_type::UTF16LE,
            std::make_pair(
                [this](const std::vector<byte>& bytes) { return bytes_to_wstring(bytes, bom_utf16le.size()); },
                [this](const std::wstring& wstr)
                {
                    std::vector<byte> bom = bom_utf16le;
                    std::vector<byte> bytes = wstring_to_bytes(wstr);
                    bytes.insert(bytes.begin(), bom.begin(), bom.end());
                    return bytes;
                }
            )},
            {encoding_type::UTF16LEN,
            std::make_pair(
                [this](const std::vector<byte>& bytes) { return bytes_to_wstring(bytes, bom_non.size()); },
                [this](const std::wstring& wstr) { return wstring_to_bytes(wstr); }
            )},
            {encoding_type::UTF8,
            std::make_pair(
                [this](const std::vector<byte>& bytes) { return utf8_to_wide(bytes_to_string(bytes, bom_utf8.size())); },
                [this](const std::wstring& wstr) 
                { 
                    std::vector<byte> bom = bom_utf8;
                    std::vector<byte> bytes = string_to_bytes(wide_to_utf8(wstr));
                    bytes.insert(bytes.begin(), bom.begin(), bom.end());
                    return bytes;
                }
            )},
            {encoding_type::UTF8N,
            std::make_pair(
                [this](const std::vector<byte>& bytes) { return utf8_to_wide(bytes_to_string(bytes, bom_non.size())); },
                [this](const std::wstring& wstr) { return string_to_bytes(wide_to_utf8(wstr)); }
            )},
            {encoding_type::ASCII,
            std::make_pair(
                [this](const std::vector<byte>& bytes) { return utf8_to_wide(bytes_to_string(bytes, bom_non.size())); },
                [this](const std::wstring& wstr) { return string_to_bytes(wide_to_utf8(wstr)); }
            )},
            {encoding_type::SJIS,
            std::make_pair(
                [this](const std::vector<byte>& bytes) { return sjis_to_wide(bytes_to_string(bytes, bom_non.size())); },
                [this](const std::wstring& wstr) { return string_to_bytes(wide_to_sjis(wstr)); }
            )}
        })
{}


std::wstring CTextEnDecoder::Decode(const std::vector<byte>& str, const encoding_type& enc)
{
    auto iter = m_map.find(enc);
    if (iter != m_map.end()) {
        return iter->second.first(str);
    } else {
        return std::wstring();
    }
}
std::vector<byte> CTextEnDecoder::Encode(const std::wstring& wstr, const encoding_type& enc)
{
    auto iter = m_map.find(enc);
    if (iter != m_map.end()) {
        return iter->second.second(wstr);
    } else {
        return std::vector<byte>();
    }
}


encoding_type CTextEnDecoder::DetectEncoding(const std::vector<byte>& str)
{
    if (str.empty()) { return encoding_type::UTF8N; }
    if (encoding_type enc = DetectEncodingByBOM(str); enc != encoding_type::UNKNOWN) { return enc; }
    if (encoding_type enc = DetectEncodingByParse(str); enc != encoding_type::UNKNOWN) { return enc; }
    if (encoding_type enc = DetectEncodingByMultiLanguage(str); enc != encoding_type::UNKNOWN) { return enc; }

    return encoding_type::UNKNOWN;
}

encoding_type CTextEnDecoder::DetectEncodingByBOM(const std::vector<byte>& str)
{
	if (str[0] == bom_utf16be[0] && str[1] == bom_utf16be[1]){
		return encoding_type::UTF16BE;
	} else if (str[0] == bom_utf16le[0] && str[1] == bom_utf16le[1]) {
		return encoding_type::UTF16LE;
	} else if (str[0] == bom_utf8[0] && str[1] == bom_utf8[1] && str[2] == bom_utf8[2]) {
		return encoding_type::UTF8;
	} else {
		return encoding_type::UNKNOWN;
	}
}

encoding_type CTextEnDecoder::DetectEncodingByParse(const std::vector<byte>& str)
{
    const byte bEscape = 0x1B;
    const byte bAt = 0x40;
    const byte bDollar = 0x24;
    const byte bAnd = 0x26;
    const byte bOpen = 0x28;    //'('
    const byte bB = 0x42;
    const byte bD = 0x44;
    const byte bJ = 0x4A;
    const byte bI = 0x49;

    int len = str.size();
    byte b1, b2, b3, b4;

    //Encode::is_utf8 ‚Í–³Ž‹

    bool isBinary = false;
    for (int i = 0; i < len; i++)
    {
        b1 = str[i];
        if (b1 <= 0x06 || b1 == 0x7F || b1 == 0xFF)
        {
            //'binary'
            isBinary = true;
            if (b1 == 0x00 && i < len - 1 && str[i + 1] <= 0x7F)
            {
                //smells like raw unicode
                return encoding_type::UTF16LEN;
            }
        }
    }
    if (isBinary)
    {
        return encoding_type::UNKNOWN;
    }

    //not Japanese
    bool notJapanese = true;
    for (int i = 0; i < len; i++)
    {
        b1 = str[i];
        if (b1 == bEscape || 0x80 <= b1)
        {
            notJapanese = false;
            break;
        }
    }
    if (notJapanese)
    {
        return encoding_type::ASCII;
    }

    for (int i = 0; i < len - 2; i++)
    {
        b1 = str[i];
        b2 = str[i + 1];
        b3 = str[i + 2];

        if (b1 == bEscape)
        {
            if (b2 == bDollar && b3 == bAt)
            {
                //JIS_0208 1978
                //JIS
                return encoding_type::JIS;
            }
            else if (b2 == bDollar && b3 == bB)
            {
                //JIS_0208 1983
                //JIS
                return encoding_type::JIS;
            }
            else if (b2 == bOpen && (b3 == bB || b3 == bJ))
            {
                //JIS_ASC
                //JIS
                return encoding_type::JIS;
            }
            else if (b2 == bOpen && b3 == bI)
            {
                //JIS_KANA
                //JIS
                return encoding_type::JIS;
            }
            if (i < len - 3)
            {
                b4 = str[i + 3];
                if (b2 == bDollar && b3 == bOpen && b4 == bD)
                {
                    //JIS_0212
                    //JIS
                    return encoding_type::JIS;
                }
                if (i < len - 5 &&
                    b2 == bAnd && b3 == bAt && b4 == bEscape &&
                    str[i + 4] == bDollar && str[i + 5] == bB)
                {
                    //JIS_0208 1990
                    //JIS
                    return encoding_type::JIS;
                }
            }
        }
    }

    //should be euc|sjis|utf8
    //use of (?:) by Hiroki Ohzaki <ohzaki@iod.ricoh.co.jp>
    int sjis = 0;
    int euc = 0;
    int utf8 = 0;
    for (int i = 0; i < len - 1; i++)
    {
        b1 = str[i];
        b2 = str[i + 1];
        if (((0x81 <= b1 && b1 <= 0x9F) || (0xE0 <= b1 && b1 <= 0xFC)) &&
            ((0x40 <= b2 && b2 <= 0x7E) || (0x80 <= b2 && b2 <= 0xFC)))
        {
            //SJIS_C
            sjis += 2;
            i++;
        }
    }
    for (int i = 0; i < len - 1; i++)
    {
        b1 = str[i];
        b2 = str[i + 1];
        if (((0xA1 <= b1 && b1 <= 0xFE) && (0xA1 <= b2 && b2 <= 0xFE)) ||
            (b1 == 0x8E && (0xA1 <= b2 && b2 <= 0xDF)))
        {
            //EUC_C
            //EUC_KANA
            euc += 2;
            i++;
        }
        else if (i < len - 2)
        {
            b3 = str[i + 2];
            if (b1 == 0x8F && (0xA1 <= b2 && b2 <= 0xFE) &&
                (0xA1 <= b3 && b3 <= 0xFE))
            {
                //EUC_0212
                euc += 3;
                i += 2;
            }
        }
    }
    for (int i = 0; i < len - 1; i++)
    {
        b1 = str[i];
        b2 = str[i + 1];
        if ((0xC0 <= b1 && b1 <= 0xDF) && (0x80 <= b2 && b2 <= 0xBF))
        {
            //UTF8
            utf8 += 2;
            i++;
        }
        else if (i < len - 2)
        {
            b3 = str[i + 2];
            if ((0xE0 <= b1 && b1 <= 0xEF) && (0x80 <= b2 && b2 <= 0xBF) &&
                (0x80 <= b3 && b3 <= 0xBF))
            {
                //UTF8
                utf8 += 3;
                i += 2;
            }
        }
    }
    //M. Takahashi's suggestion
    //utf8 += utf8 / 2;

    //System.Diagnostics.Debug.WriteLine(
    //    string.Format("sjis = {0}, euc = {1}, utf8 = {2}", sjis, euc, utf8));
    if (euc > sjis && euc > utf8)
    {
        //EUC
        return encoding_type::EUC;
    }
    else if (sjis > euc && sjis > utf8)
    {
        //SJIS
        return encoding_type::SJIS;
    }
    else if (utf8 > euc && utf8 > sjis)
    {
        //UTF8
        return encoding_type::UTF8N;
    }

    return encoding_type::UNKNOWN;
}

encoding_type CTextEnDecoder::DetectEncodingByMultiLanguage(const std::vector<byte>& str)
{
	CComPtr<IMultiLanguage3> p;
	FAILED_THROW(::CoCreateInstance(CLSID_CMultiLanguage, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p)));
	DetectEncodingInfo encoding = {};
	INT size = str.size();
	INT scores = 1;
	FAILED_THROW(p->DetectInputCodepage(MLDETECTCP_NONE, 0, (char*)(str.data()), &size, &encoding, &scores));
	switch (encoding.nCodePage) {
		case 932:
			return encoding_type::SJIS;
		case 20127:
			return encoding_type::ASCII;
		case 50220:
			return encoding_type::JIS;
		case 51932:
			return encoding_type::EUC;
		case 65001:
		default:
			return encoding_type::UTF8N;
	}

}

//std::pair<encoding_type, std::wstring> parse_ifstream(std::ifstream& ifs)
//{
//	static std::array<BYTE, 3> bom_utf8 = {0xEF, 0xBB, 0xBF};
//	static std::array<BYTE, 3> bom_utf16le = { 0xFF, 0xFE, 0x00 };
//	static std::array<BYTE, 3> bom_utf16be = { 0xFE, 0xFF, 0x00 };
//	static INT test = IS_TEXT_UNICODE_STATISTICS;
//
//	std::array<BYTE, 3> bom;
//	bom[0] = ifs.get();
//	bom[1] = ifs.get();
//	bom[2] = ifs.get();
//	ifs.seekg(0);
//
//	if (bom[0] == bom_utf16be[0] && bom[1] == bom_utf16be[1]){
//		throw std::exception("UTF16BE is not supported");
//	} else if (bom[0] == bom_utf16le[0] && bom[1] == bom_utf16le[1]) {
//		return {encoding_type::utf16le, std::wstring(std::next(std::istreambuf_iterator<char>(ifs), 2), std::istreambuf_iterator<char>()) };
//	} else if (bom[0] == bom_utf8[0] && bom[1] == bom_utf8[1] && bom[2] == bom_utf8[2]) {
//		return {encoding_type::utf8, utf8_to_wide(std::string(std::next(std::istreambuf_iterator<char>(ifs), 3), std::istreambuf_iterator<char>())) };
//	} else {
//		auto str = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
//
//		//if (::IsTextUnicode(str.c_str(), str.size(), &test)) {
//		//	return { encoding_type::utf16len, std::wstring(str.begin(), str.end()) };
//		//} else {
//			CComPtr<IMultiLanguage3> p;
//			FAILED_THROW(::CoCreateInstance(CLSID_CMultiLanguage, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p)));
//			DetectEncodingInfo encoding = {};
//			INT size = str.size();
//			INT scores = 1;
//			p->DetectInputCodepage(MLDETECTCP_NONE, 0, const_cast<CHAR*>(str.c_str()), &size, &encoding, &scores);
//			switch (encoding.nCodePage) {
//				case 932:
//				return { encoding_type::sjis, sjis_to_wide(str) };
//				case 65001:
//				default:
//				return { encoding_type::utf8n, cp_to_wide(str, encoding.nCodePage) };
//			}
//		//}
//	}
//}
