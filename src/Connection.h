#pragma once

#include <array>
#include <memory>
#include <asio/asio.hpp>
#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"
#include "RequestParser.h"

class ConnectionManager;

class Connection
  : public std::enable_shared_from_this<Connection>
{
public:
  explicit Connection(asio::ip::tcp::socket socket,
      ConnectionManager& manager, RequestHandler& handler);

  void Start();

  void Stop();

private:
  void Read();

  void Write();

  asio::ip::tcp::socket socket_;
  ConnectionManager& connection_manager_;
  RequestHandler& request_handler_;
  std::array<char, 8192> buffer_;
  Request request_;
  RequestParser request_parser_;
  Reply reply_;
};

typedef std::shared_ptr<Connection> connection_ptr;