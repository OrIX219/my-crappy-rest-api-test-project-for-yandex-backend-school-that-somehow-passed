#include "Server.h"
#include <signal.h>
#include <utility>
#include <iostream>

Server::Server(const std::string& address, 
  const std::string& port, RequestHandler request_handler)
  : io_service_(),
  signals_(io_service_),
  acceptor_(io_service_),
  connection_manager_(),
  request_handler_(std::move(request_handler)),
  socket_(io_service_) {
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif
  AwaitStop();

  asio::ip::tcp::resolver resolver(io_service_);
  asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  Accept();
}

void Server::Run() {
  io_service_.run();
}

void Server::Accept() {
  acceptor_.async_accept(socket_,
    [this](asio::error_code ec)
    {
      if (!acceptor_.is_open()) {
        return;
      }

      if (!ec) {
        connection_manager_.Start(std::make_shared<Connection>(
          std::move(socket_), connection_manager_, request_handler_));
      } else {
        std::cerr << ec.message() << std::endl;
      }

      Accept();
    });
}

void Server::AwaitStop()
{
  signals_.async_wait(
    [this](asio::error_code /*ec*/, int /*signo*/)
    {
      acceptor_.close();
      connection_manager_.StopAll();
    });
}