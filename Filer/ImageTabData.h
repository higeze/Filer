#pragma once

#include "TabControl.h"
#include "D2DImage.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"



/***************/
/* ImageTabData */
/***************/
struct ImageTabData :public TabData
{
	//ReactiveWStringProperty Path;
	reactive_property_ptr<CD2DImage> Image;
	reactive_property_ptr<FLOAT> VScroll;
	reactive_property_ptr<FLOAT> HScroll;
	reactive_property_ptr<FLOAT> Scale;

	//ReactiveCommand<HWND> OpenCommand;
	//ReactiveCommand<HWND> SaveCommand;
	//ReactiveCommand<HWND> OpenAsCommand;
	//ReactiveCommand<HWND> SaveAsCommand;


	ImageTabData(const std::wstring& path = std::wstring())
		:TabData(),Image(path),
		VScroll(0.f),
		HScroll(0.f),
		Scale(-1.f)
	{
		
		//OpenCommand.Subscribe([this](HWND hWnd) { Open(hWnd); });
		//SaveCommand.Subscribe([this](HWND hWnd) { Save(hWnd); });
		//OpenAsCommand.Subscribe([this](HWND hWnd) { OpenAs(hWnd); });
		//SaveAsCommand.Subscribe([this](HWND hWnd) { SaveAs(hWnd); });
	}



	virtual ~ImageTabData() = default;

	ImageTabData(const ImageTabData& other)
		:ImageTabData(other.Image->GetPath()){}
	virtual std::shared_ptr<TabData> ClonePtr() const override { return std::make_shared<ImageTabData>(*this); }


	//void Open(HWND hWnd);
	//void OpenAs(HWND hWnd);
	//void Open(const std::wstring& path, const encoding_type& enc);

	//void Save(HWND hWnd);
	//void SaveAs(HWND hWnd);
	//void Save(const std::wstring& path, const encoding_type& enc);

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override { return true; }

	template<class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::base_class<TabData>(this));
		archive(CEREAL_NVP(Image));
	}
	template<class Archive>
	void load(Archive& archive)
	{
		archive(cereal::base_class<TabData>(this));
		archive(CEREAL_NVP(Image));
	}

	friend void to_json(json& j, const ImageTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Image"] = o.Image;
	}
	friend void from_json(const json& j, ImageTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Image").get_to(o.Image);
	}
};

