#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Header.h"
#include "json/json.hpp"

struct Request {
  std::string method;
  std::string uri;
  std::unordered_map<std::string, std::string> params;
  nlohmann::json body;
};