#pragma once
#include "TabControl.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"

/************************/
/* ThreadMonitorTabData */
/************************/

struct ThreadMonitorTabData :public TabData
{
	ThreadMonitorTabData()
		:TabData(){}

	virtual ~ThreadMonitorTabData() = default;

	ThreadMonitorTabData(const ThreadMonitorTabData& other)
		:ThreadMonitorTabData(){}

	virtual std::shared_ptr<TabData> ClonePtr() const override { return std::make_shared<ThreadMonitorTabData>(*this); }

public:
};

JSON_ENTRY_TYPE(TabData, ThreadMonitorTabData)



