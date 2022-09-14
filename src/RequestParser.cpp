#include "RequestParser.h"

#include "json/json.hpp"
#include <iostream>
#include "Utils.h"

size_t RequestParser::GetContentLength(std::string_view str) {
  size_t cl = str.find("Content-Length");
  size_t colon = str.find_first_of(":", cl);
  size_t new_line = str.find_first_of("\n", colon + 1);
  return std::atoi(str.substr(colon + 2, new_line).data());
}

std::unordered_map<std::string, std::string>
  RequestParser::ParseParameters(std::string_view str) {
  std::unordered_map<std::string, std::string> params;
  size_t find_pos = str.find_first_of("?");
  if (find_pos == str.npos)
    return {};
  str.remove_prefix(find_pos + 1);
  std::string key, value;
  while (!str.empty()) {
    find_pos = str.find_first_of("=");
    key = str.substr(0, find_pos);
    if (find_pos == str.npos || key.size() == 0)
      return {};
    str.remove_prefix(find_pos + 1);
    find_pos = str.find_first_of("&");
    value = str.substr(0, find_pos);
    if (value.size() == 0 || value == " ")
      return {};
    str.remove_prefix(find_pos != str.npos 
      ? find_pos + 1 : str.size());
    params[key] = utils::UrlDecode(value);
  }

  return params;
}

bool RequestParser::ParseBody(Request& req, std::string_view str) {
  size_t body_start = str.find_first_of("{");
  if (body_start == str.npos)
    return false;
  str = str.substr(body_start, str.find_last_of("}") + 1 - body_start);
  req.body = json::parse(str, nullptr, false);
  return !req.body.is_discarded();
}

void RequestParser::ParseHead(Request& req, std::string_view str) {
  size_t find_pos = str.find_first_of(" ");
  req.method = str.substr(0, find_pos);
  str.remove_prefix(find_pos + 1);
  find_pos = str.find_first_of(" ");
  req.uri = str.substr(1, find_pos);
  str.remove_prefix(find_pos + 1);
  if (req.method == "GET" || req.method == "DELETE") {
    req.params = ParseParameters(req.uri);
    req.uri = req.uri.substr(0, req.uri.find_first_of("?"));
  }
}