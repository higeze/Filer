#pragma once
#include <sigslot/signal.hpp>

class NotifyPropertyChanged
{
public:
	sigslot::signal<std::string> PropertyChanged;
protected: 
    void RaisePropertyChanged(std::string name)
    {
        PropertyChanged(name);
    }
};
