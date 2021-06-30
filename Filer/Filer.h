#include "ApplicationProperty.h"
#include "FilerProperty.h"

void SerializeProperty(CFilerWnd* pProp);


enum class json_path
{
	main = 0,
	favorites = 1,
	launcher = 2,
	exeextension = 3,
};

std::tuple<std::string, std::string, std::string, std::string> GetJsonPaths();



