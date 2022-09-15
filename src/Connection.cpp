#include "Connection.h"
#include <utility>
#include <vector>
#include <iostream>
#include "ConnectionManager.h"
#include "RequestHandler.h"

Connection::Connection(asio::ip::tcp::socket socket,
    ConnectionManager& manager, RequestHandler& handler)
  : socket_(std::move(socket)),
    connection_manager_(manager),
    request_handler_(handler) {}

void Connection::Start() {
  Read();
}

void Connection::Stop() {
  socket_.close();
}

void Connection::Read() {
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer_),
    [this, self](asio::error_code ec, size_t bytes_transferred) {
      if (!ec) {
        request_parser_.ParseHead(request_, {buffer_.data(), bytes_transferred});
        bool body_correct = true;
        if (request_.method == "POST") {
          std::string_view str(buffer_.data());
          if (str.rfind("Z\"") != str.npos) {
            body_correct = request_parser_.ParseBody(request_, str);
          } else {
            size_t content_length = request_parser_.GetContentLength(buffer_.data());
            asio::read(socket_, asio::buffer(buffer_), 
              asio::transfer_exactly(content_length));
            body_correct = 
              request_parser_.ParseBody(request_, {buffer_.data(), content_length});
          }
        }
        if (body_correct) {
          request_handler_.HandleRequest(request_, reply_);
          Write();
        } else {
          reply_ = Reply::StockReply(Reply::bad_request);
          Write();
        }
      } else if (ec != asio::error::operation_aborted) {
        connection_manager_.Stop(shared_from_this());
      } else {
        std::cerr << ec.message() << std::endl;
      }
    });
}

void Connection::Write() {
  auto self(shared_from_this());
  asio::async_write(socket_, reply_.ToBuffers(),
    [this, self](asio::error_code ec, size_t) {
      if (!ec) {
        asio::error_code ignored_ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both,
          ignored_ec);
      } else if (ec != asio::error::operation_aborted) {
        connection_manager_.Stop(shared_from_this());
      }
    });
}