#include "TextDoc.h"
#include "MyFile.h"
#include "TextEnDecoder.h"

CTextDoc::CTextDoc(const std::wstring& path, const encoding_type& encoding)
	:Path(path),
	Encoding(encoding),
	Text(L""),
	Status(FileStatus::None),
	//Caret(0, 0, 0, 0, 0),
	Dummy(std::make_shared<int>(0))
{
	Text.subscribe([sta = Status](auto) mutable
	{
		sta.set(FileStatus::Dirty);
	}, Status.life());
}

void CTextDoc::Open(const std::wstring& path, const encoding_type& encode)
{
	if (::PathFileExists(path.c_str())) {
		Path.set(path);

		std::vector<byte> bytes = CFile::ReadAllBytes(path);
		encoding_type enc = (encode == encoding_type::UNKNOWN) ? CTextEnDecoder::GetInstance()->DetectEncoding(bytes) : encode;
		std::wstring wstr = CTextEnDecoder::GetInstance()->Decode(bytes, enc);
		auto iter = std::remove_if(wstr.begin(), wstr.end(), [](const auto& c)->bool { return c == L'\r'; });
		wstr.erase(iter, wstr.end());

		//std::ifstream ifs(path);
		//std::string str = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		//encoding_type enc = CTextEnDecoder::GetInstance()->DetectEncoding(str);
		//std::wstring wstr = CTextEnDecoder::GetInstance()->Decode(str, enc);

		Encoding.set(enc);
		Text.assign(wstr);
		Status.force_notify_set(FileStatus::Saved);
		//Caret.get_unconst()->Set(0, 0, 0, 0, 0, CPointF(0, 10 * 0.5f));//TODOLOW
	} else {
		Path.set(L"");
		Status.force_notify_set(FileStatus::Saved);
		//Caret.get_unconst()->Set(0, 0, 0, 0, 0, CPointF(0, 10 * 0.5f));//TODOLOW
	}
}

void CTextDoc::Close()
{
	Path.set(L"");
	Text.set(L"");
	Encoding.set(encoding_type::UNKNOWN);
	//Caret.set(CTextCaret());
	Status.set(FileStatus::None);
}

void CTextDoc::Save(const std::wstring& path, const encoding_type& encoding)
{
	Path.set(path);
	Encoding.set(encoding);
	Status.force_notify_set(FileStatus::Saved);
	std::ofstream ofs(path);
	std::vector<byte> bytes = CTextEnDecoder::GetInstance()->Encode(*Text, encoding);
	CFile::WriteAllBytes(*Path, bytes);
}