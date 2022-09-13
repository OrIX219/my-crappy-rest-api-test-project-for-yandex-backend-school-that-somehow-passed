#pragma once

#include <algorithm>
#include <string>
#include <sstream>
#include "date/date.h"

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

}