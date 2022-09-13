#include <string>
#include <vector>
#include <asio/asio.hpp>
#include "Header.h"


struct Reply
{
  enum status_type {
    ok = 200,
    bad_request = 400,
    not_found = 404
  } status;

  std::string content;

  std::vector<asio::const_buffer> ToBuffers();

  static Reply StockReply(status_type status);
};