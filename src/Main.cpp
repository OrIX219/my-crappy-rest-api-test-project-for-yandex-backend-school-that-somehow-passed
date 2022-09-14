#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>
#include <asio/asio.hpp>
#include "Server.h"
#include "DbManager.h"
#include "Utils.h"

bool ImportsValidate(DbManager& dbm, const json& req) {
  if (!req.contains("items")
    || !req.contains("updateDate"))
    return false;
  if (!utils::IsDateValid(req["updateDate"].get<std::string>())) 
    return false;
  if (!req["items"].is_array() || req["items"].empty())
    return false;

  std::unordered_set<std::string> ids;
  std::unordered_set<std::string> folders;
  for (const auto& item : req["items"]) {
    if (!item.contains("id")
      || !item.contains("type")
      || !item.contains("parentId"))
      return false;
    const json& id = item["id"];
    const json& type = item["type"];
    const json& parentId = item["parentId"];

    if (id.is_null() || type.is_null()
      || ids.count(id.get<std::string>()) > 0)
      return false;
    ids.insert(id.get<std::string>());

    if (!parentId.is_null() 
      && db::GetType(dbm, parentId.get<std::string>()) != "FOLDER"
      && folders.count(parentId.get<std::string>()) == 0) {
      return false;
    }

    std::string type_str = type.get<std::string>();
    if (type_str == "FOLDER") {
      if (item.contains("size") && !item.at("size").is_null())
        return false;
      if (item.contains("url") && !item.at("url").is_null())
        return false;
      folders.insert(id.get<std::string>());
    } else if (type_str == "FILE") {
      if (!item.contains("size") || item["size"].get<int>() <= 0)
        return false;
      if (!item.contains("url") 
        || item["url"].get<std::string>().size() > 255)
        return false;
    } else {
      return false;
    }
  }

  return true;
}

void ChangeSize(DbManager& dbm, const std::string& id, int delta, const std::string& date) {
  std::string parent = db::GetParent(dbm, id);
  if (!parent.empty())
    ChangeSize(dbm, parent, delta, date);
  db::ChangeSize(dbm, id, delta);
  db::SetDate(dbm, id, date);
  json updatedFolder = db::GetItem(dbm, id);
  db::InsertIntoHistory(dbm, updatedFolder);
}

void ImportsHandler(DbManager& dbm, const Request& req, Reply& rep) {
  if (!ImportsValidate(dbm, req.body)) {
    rep = Reply::StockReply(Reply::bad_request);
    return;
  }

  json body = req.body;
  std::string date = body["updateDate"].get<std::string>();
  for (json& item : body["items"]) {
    item["date"] = date;
    if (item["type"].get<std::string>() == "FOLDER") {
      item["size"] = 0;
      item["url"] = json::value_t::null;
    }
    const std::string& id = item["id"].get<std::string>();
    int size_delta;
    std::string old_parent;
    if (!db::ItemExists(dbm, id)) {
      size_delta = db::InsertItem(dbm, item);
    } else {
      old_parent = db::GetParent(dbm, id);
      size_delta = db::UpdateItem(dbm, item);
    }
    std::string parent = db::GetParent(dbm, id);
    if (old_parent != parent) {
      size_delta = item["size"].get<int>();
      if (!old_parent.empty())
        ChangeSize(dbm, old_parent, -item["size"].get<int>(), date);
    }
    if (!parent.empty())
      ChangeSize(dbm, parent, size_delta, date);
  }
  rep.status = Reply::ok;
}

int DeleteIter(DbManager& dbm, const std::string& id) {
  int deleted_size = 0;
  for (const std::string& child_id : db::GetChildren(dbm, id))
    deleted_size += DeleteIter(dbm, child_id);
  bool is_file = db::GetType(dbm, id) == "FILE";
  int cur_size = db::DeleteItem(dbm, id);
  if (is_file)
    deleted_size += cur_size;
  
  db::DeleteFromHistory(dbm, id);
  return deleted_size;
}

void DeleteHandler(DbManager& dbm, const Request& req, Reply& rep) {
  if (req.params.count("date") == 0
    || !utils::IsDateValid(req.params.at("date"))) {
    rep = Reply::StockReply(Reply::bad_request);
    return;
  }    
  std::string id = 
    req.uri.substr(req.uri.find_first_of("/") + 1);
  if (!db::ItemExists(dbm, id)) {
    rep = Reply::StockReply(Reply::not_found);
    return;
  }

  std::string parent = db::GetParent(dbm, id);
  int deleted_size = DeleteIter(dbm, id);
  if (!parent.empty())
    ChangeSize(dbm, parent, -deleted_size, req.params.at("date"));

  rep.status = Reply::ok;
}

json NodesIter(DbManager& dbm, const std::string& id) {
  json item = db::GetItem(dbm, id);
  if (item["type"].get<std::string>() == "FOLDER") {
    item["children"] = json::value_t::array;
    for (const std::string& child : db::GetChildren(dbm, id))
      item["children"].push_back(NodesIter(dbm, child));
  } else {
    item["children"] = json::value_t::null;
  }

  return item;
}

void NodesHandler(DbManager& dbm, const Request& req, Reply& rep) {
  std::string id = 
    req.uri.substr(req.uri.find_first_of("/") + 1);
  utils::StripSpaces(id);
  if (!db::ItemExists(dbm, id)) {
    rep = Reply::StockReply(Reply::not_found);
    return;
  }

  json item = NodesIter(dbm, id);
  rep.content = item.dump(2, ' ');
  rep.status = Reply::ok;
}

void UpdatesHandler(DbManager& dbm, const Request& req, Reply& rep) {
  if (req.params.count("date") == 0
    || !utils::IsDateValid(req.params.at("date"))) {
    rep = Reply::StockReply(Reply::bad_request);
    return;
  } 

  json res;
  res["items"] = json::value_t::array;
  std::string date24ago = utils::Date24HAgo(req.params.at("date"));
  for (const std::string& item : 
    db::GetUpdatedBetween(dbm, date24ago, req.params.at("date"))) {
    res["items"].push_back(db::GetItem(dbm, item));
  }

  rep.content = res.dump(2, ' ');
  rep.status = Reply::ok;
}

void HistoryHandler(DbManager& dbm, const Request& req, Reply& rep) {
  size_t id_start = req.uri.find_first_of("/") + 1;
  std::string id = req.uri.substr(
    id_start, req.uri.find_last_of("/") - id_start);
  if (!db::ItemExists(dbm, id)) {
    rep = Reply::StockReply(Reply::not_found);
    return;
  }
  if (req.params.count("dateStart") == 0
    || !utils::IsDateValid(req.params.at("dateStart"))) {
    std::cerr << req.params.at("dateStart");
    rep = Reply::StockReply(Reply::bad_request);
    return;
  }
  if (req.params.count("dateEnd") == 0
    || !utils::IsDateValid(req.params.at("dateEnd"))) {
    rep = Reply::StockReply(Reply::bad_request);
    return;
  }  

  json res = db::GetHistoryBetween(dbm, id,
    req.params.at("dateStart"), req.params.at("dateEnd"));

  rep.content = res.dump(2, ' ');
  rep.status = Reply::ok;
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 5) {
      std::cerr << "Usage:\n";
      std::cerr << "\tMain <address> <port> <db name> <db user>\n";
      return 1;
    }

    DbManager dbm(argv[3], argv[4]);

    db::CreateItemTable(dbm);
    db::CreateHistoryTable(dbm);

    RequestHandler request_handler;
    request_handler.SetHandler("imports", 
      std::bind(ImportsHandler, std::ref(dbm), 
      std::placeholders::_1, std::placeholders::_2));
    request_handler.SetHandler("delete", 
      std::bind(DeleteHandler, std::ref(dbm), 
      std::placeholders::_1, std::placeholders::_2));
    request_handler.SetHandler("nodes", 
      std::bind(NodesHandler, std::ref(dbm), 
      std::placeholders::_1, std::placeholders::_2));
    request_handler.SetHandler("updates", 
      std::bind(UpdatesHandler, std::ref(dbm), 
      std::placeholders::_1, std::placeholders::_2));
    request_handler.SetHandler("node", 
      std::bind(HistoryHandler, std::ref(dbm), 
      std::placeholders::_1, std::placeholders::_2));
      
    Server s(argv[1], argv[2], std::move(request_handler));

    s.Run();
  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}