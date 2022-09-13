#include "Reply.h"
#include <string>

namespace status_strings {

const std::string ok =
  "HTTP/1.1 200 OK\r\n";
const std::string bad_request =
  "HTTP/1.1 400 Bad Request\r\n";
const std::string not_found =
  "HTTP/1.1 404 Not Found\r\n";

asio::const_buffer ToBuffer(Reply::status_type status) {
  switch (status) {
  case Reply::ok:
    return asio::buffer(ok);
  case Reply::bad_request:
    return asio::buffer(bad_request);
  case Reply::not_found:
    return asio::buffer(not_found);
  }
}

}

namespace misc_strings {

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

}

std::vector<asio::const_buffer> Reply::ToBuffers() {
  std::vector<asio::const_buffer> buffers;
  buffers.push_back(status_strings::ToBuffer(status));
  buffers.push_back(asio::buffer(misc_strings::crlf));
  buffers.push_back(asio::buffer(content));
  return buffers;
}

namespace stock_replies {

const char ok[] = "";
const char bad_request[] =
"{\n"
"  \"code\": 400,\n"
"  \"message\": \"Validation Failed\"\n"
"}";
const char not_found[] =
"{\n"
"  \"code\": 404,\n"
"  \"message\": \"Item not found\"\n"
"}";

std::string ToString(Reply::status_type status) {
  switch (status) {
  case Reply::ok:
    return ok;
  case Reply::bad_request:
    return bad_request;
  case Reply::not_found:
    return not_found;
  }
}

}

Reply Reply::StockReply(Reply::status_type status) {
  Reply rep;
  rep.status = status;
  rep.content = stock_replies::ToString(status);
  return rep;
}