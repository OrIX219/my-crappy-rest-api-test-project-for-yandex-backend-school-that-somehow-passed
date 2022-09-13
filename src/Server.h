#pragma once

#include <asio/asio.hpp>
#include <string>
#include "Connection.h"
#include "ConnectionManager.h"
#include "RequestHandler.h"

class Server
{
public:
  Server(const Server& other) = delete;
  Server& operator=(const Server& other) = delete;

  explicit Server(const std::string& address, 
    const std::string& port, RequestHandler request_handler);

  void Run();

private:
  void Accept();

  void AwaitStop();

  asio::io_service io_service_;
  asio::signal_set signals_;
  asio::ip::tcp::acceptor acceptor_;
  ConnectionManager connection_manager_;
  asio::ip::tcp::socket socket_;
  RequestHandler request_handler_;
};