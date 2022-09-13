#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include "Request.h"
#include "json/json.hpp"

using nlohmann::json;

struct Request;

class RequestParser {
public:
  size_t GetContentLength(std::string_view str);
  std::unordered_map<std::string, std::string>
    ParseParameters(std::string_view str);
  bool ParseBody(Request& req, std::string_view str);
  void ParseHead(Request& req, std::string_view str);
};