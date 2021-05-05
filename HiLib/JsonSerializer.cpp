#include "JsonSerializer.h"

std::unordered_map<std::string,
    std::pair<
        std::function<void(const json&, std::shared_ptr<void>&)>,// from
        std::function<void(json&, const std::shared_ptr<const void>&)> //to
    >>  json_polymorphic_map;

std::unordered_map <
    std::string,
    std::function<std::shared_ptr<void>()>> json_make_shared_map;
