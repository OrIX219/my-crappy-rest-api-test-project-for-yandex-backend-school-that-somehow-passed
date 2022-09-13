#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

struct Reply;
struct Request;

class RequestHandler {
public:
  explicit RequestHandler();

  void HandleRequest(const Request& req, Reply& rep);

  typedef std::function<void(const Request& req, Reply& rep)> handler_t;

  void SetHandler(std::string_view command, handler_t handler);

private:
  std::unordered_map<std::string_view, handler_t> handlers_;
};