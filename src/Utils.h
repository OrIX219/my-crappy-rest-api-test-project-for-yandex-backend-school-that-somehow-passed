#pragma once

#include <algorithm>
#include <string>
#include <sstream>
#include <date/date.h>
#include <json/json.hpp>
#include <pqxx/pqxx>

using nlohmann::json;

namespace utils {

static void StripSpaces(std::string& str) {
  str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
}

static bool IsDateValid(const std::string& date) {
  std::istringstream in(date);
  date::local_seconds temp;
  in >> date::parse("%FT%TZ", temp);
  return !in.fail();
}

static std::string Date24HAgo(const std::string& date) {
  std::istringstream in(date);
  date::local_seconds cur;
  in >> date::parse("%FT%TZ", cur);
  date::local_seconds past = cur - date::days(1);
  return date::format("%FT%TZ", past);
}

static std::string UrlDecode(const std::string& in) {
  std::string out;
  out.reserve(in.size());
  for (size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        } else {
          return "";
        }
      } else {
        return "";
      }
    } else if (in[i] == '+') {
      out += ' ';
    } else {
      out += in[i];
    }
  }
  return out;
}

static json MakeItemFromRow(const pqxx::row& row) {
  json item;
  item["id"] = row["id"].as<std::string>();
  item["type"] = row["type"].as<std::string>();
  item["size"] = row["size"].as<int>();
  item["date"] = row["date"].as<std::string>();
  if (row["parentId"].is_null())
    item["parentId"] = json::value_t::null;
  else
    item["parentId"] = row["parentId"].as<std::string>();
  if (row["url"].is_null())
    item["url"] = json::value_t::null;
  else
    item["url"] = row["url"].as<std::string>(); 

  return item;
}

}