#include "RequestHandler.h"

#include <algorithm>
#include "Reply.h"
#include "Request.h"
#include "Utils.h"

RequestHandler::RequestHandler() {}

void RequestHandler::HandleRequest(const Request& req, Reply& rep) {
  std::string command = req.uri.substr(0, req.uri.find_first_of("/"));
  utils::StripSpaces(command);
  if (handlers_.find(command) != handlers_.end()) {
    handlers_[command](req, rep);
  } else {
    rep = Reply::StockReply(Reply::not_found);
    return;
  }
}

void RequestHandler::SetHandler(
  std::string_view command, handler_t handler) {
  handlers_[command] = handler;
}