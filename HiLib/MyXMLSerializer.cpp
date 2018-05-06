#include "MyXMLSerializer.h"

std::map<std::string, std::function<void(CSerializer*, MSXML2::IXMLDOMElementPtr, void*)>> CSerializer::s_dynamicSerializeMap;
std::map<std::string, std::function<void(CDeserializer*, MSXML2::IXMLDOMElementPtr, void*)>> CDeserializer::s_dynamicDeserializeMap;
std::map<std::string, std::function<std::shared_ptr<void>()>> CDeserializer::s_dynamicMakeSharedMap;
