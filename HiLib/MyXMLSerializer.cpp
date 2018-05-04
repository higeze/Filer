#include "MyXMLSerializer.h"

std::map<std::string, std::function<void(CSerializer*, MSXML2::IXMLDOMElementPtr, void*)>> CSerializer::s_dynamicSerializeMap;
std::map<std::string, std::function<std::shared_ptr<void>()>> CDeserializer::s_dynamicMakeSharedMap;
