#include <algorithm>
#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <vector>
#include <future>
#include <json/json.hpp>
#include "Utils.h"
#include "Paginator.h"

#define NTHREADS 4

using nlohmann::json;

class DbManager {
  public:
  DbManager(const std::string& db_name,
    const std::string& user = "root", 
    const std::string& password = "",
    const std::string& addr = "0.0.0.0",
    const std::string& port = "5432");
  
  pqxx::work Work();
  pqxx::nontransaction NonTransaction();

  private:
  pqxx::connection connection_;
};

namespace db {

static void CreateItemTable(DbManager& dbm) {
  pqxx::work work = dbm.Work();
  work.exec0(
    "CREATE TABLE IF NOT EXISTS items("
    "id TEXT PRIMARY KEY  NOT NULL,"
    "type TEXT            NOT NULL,"
    "size INT             NOT NULL,"
    "date TEXT            NOT NULL,"
    "parentId TEXT,"
    "url TEXT"
    ");"
  );
  work.commit();
}

static void CreateHistoryTable(DbManager& dbm) {
  pqxx::work work = dbm.Work();
  work.exec0(
    "CREATE TABLE IF NOT EXISTS history("
    "id TEXT              NOT NULL,"
    "type TEXT            NOT NULL,"
    "size INT             NOT NULL,"
    "date TEXT            NOT NULL,"
    "parentId TEXT,"
    "url TEXT"
    ");"
  );
  work.commit();
}

static bool ItemExists(DbManager& dbm, const std::string& id) {
  pqxx::nontransaction nt = dbm.NonTransaction();
  pqxx::result res = nt.exec("SELECT id FROM items WHERE id = '" + id + "';");
  return !res.empty();
}

static std::string MakeUpdateString(
  const std::string& table,
  std::vector<std::string> keys,
  const json& item) {
  std::stringstream str;
  std::string delim = "";
  for (const std::string& key : keys) {
    str << delim << key 
      << "=" << item[key];
    delim = ",";
  }
  std::string val = str.str();
  std::replace(val.begin(), val.end(), '"', '\'');
  str.str(std::string());
  str << "UPDATE " << table << " SET ";
  str << val << " WHERE id = '" 
    << item["id"].get<std::string>() << "';";
  return str.str();
}

static std::string MakeInsertString(
  const std::string& table,
  std::vector<std::string> keys,
  const json& item) {
  std::stringstream str, key_str;
  std::string delim = "";
  for (const std::string& key : keys) {
    str << delim << item[key];
    key_str << delim << key;
    delim = ",";
  }
  std::string val = str.str();
  std::replace(val.begin(), val.end(), '"', '\'');
  str.str(std::string());
  str << "INSERT INTO " << table << "(";
  str << key_str.str() << ") VALUES (";
  str << val << ");";
  return str.str();
}

static void InsertIntoHistory(DbManager& dbm, const json& item) {
  pqxx::work work = dbm.Work();
  work.exec0(MakeInsertString("history",
    {"id", "type","size","date","parentId","url"},
    item));
  work.commit();
}

static void DeleteFromHistory(DbManager& dbm, const std::string& id) {
  pqxx::work work = dbm.Work();;
  work.exec0("DELETE FROM history WHERE id = '" + id + "';");
  work.commit();
}

static int UpdateItem(DbManager& dbm, const json& item) {
  pqxx::work work = dbm.Work();
  pqxx::row row = work.exec1("SELECT size FROM items WHERE id = '" 
    + item["id"].get<std::string>() + "';");
  int old_size = row[0].as<int>();
  work.exec0(MakeUpdateString("items", 
    {"type","size","date","parentId","url"}, 
    item));

  work.commit();
  InsertIntoHistory(dbm, item);

  return item["size"].get<int>() - old_size;
}

static int InsertItem(DbManager& dbm, const json& item) {
  pqxx::work work = dbm.Work();
  work.exec0(MakeInsertString("items",
    {"id","type","size","date","parentId","url"}, 
    item));

  work.commit();
  InsertIntoHistory(dbm, item);

  return item["size"].get<int>();
}

static std::string GetType(DbManager& dbm, const std::string& id) {
  pqxx::nontransaction nt = dbm.NonTransaction();
  pqxx::result res = 
    nt.exec("SELECT type FROM items WHERE id = '" + id + "';");
  return res.empty() ? "" : res[0][0].as<std::string>();
}

static void ChangeSize(DbManager& dbm, const std::string& id, int delta) {
  pqxx::work work = dbm.Work();
  work.exec0("UPDATE items SET size = size + " 
    + std::to_string(delta) + " WHERE id = '"
    + id + "';");
  work.commit();
}

static void SetDate(DbManager& dbm, 
  const std::string& id, const std::string& date) {
    pqxx::work work = dbm.Work();
    work.exec0("UPDATE items SET date = '" + date 
      + "' WHERE id = '" + id + "';");
    work.commit();
}

static std::string GetParent(DbManager& dbm, const std::string& id) {
  pqxx::nontransaction nt = dbm.NonTransaction();
  pqxx::row row = nt.exec1("SELECT parentId FROM items WHERE id = '" + id + "';");
  if (row[0].is_null())
    return "";
  else
    return row[0].as<std::string>();
}

static std::vector<std::string> 
  GetChildren(DbManager& dbm, const std::string& id) {
  std::vector<std::string> children;
  pqxx::nontransaction nt = dbm.NonTransaction();
  pqxx::result res = 
    nt.exec("SELECT id FROM items WHERE parentId = '" + id + "';");
  for (const auto& row : res)
    children.push_back(row[0].as<std::string>());
  return children;
}

static int DeleteItem(DbManager& dbm, const std::string& id) {
  pqxx::work work = dbm.Work();
  pqxx::row row = work.exec1("SELECT size FROM items WHERE id = '" + id + "';");
  int size = row[0].as<int>();
  work.exec0("DELETE FROM items WHERE id = '" + id + "';");
  work.commit();
  return size;
}

static json GetItem(DbManager& dbm, const std::string& id) {
  pqxx::nontransaction nt = dbm.NonTransaction();
  pqxx::row row = nt.exec1("SELECT * FROM items WHERE id = '" + id + "';");
  json item = utils::MakeItemFromRow(row);

  return item;
}

static json GetUpdatedBetween(DbManager& dbm, 
  const std::string& date1, const std::string& date2) {
  json items;
  pqxx::nontransaction nt = dbm.NonTransaction();
  const std::string format = "YYYY-MM-DDXHH24:MI:SSX";
  pqxx::result res = nt.exec("SELECT * FROM items WHERE type = 'FILE' "
    "AND date::timestamp >= to_timestamp('" + date1 + "', '" + format + "') "
    "AND date::timestamp <= to_timestamp('" + date2 + "', '" + format + "') "
    "ORDER BY date::timestamp, id;");
  items["items"] = json::value_t::array;
  items["items"].get_ptr<json::array_t*>()->reserve(res.size());
  std::vector<std::future<void>> futures;
  size_t i = 0;
  /*
  * Here and below I should've used 
  * items["items"].size() / NTHREADS instead of just NTHREADS
  * Wtf, how could I make such a stupid mistake
  * Won't change it for historical accuracy 
  */
  for (auto page : Paginate(res, NTHREADS)) {
    futures.push_back(std::async([&, page](size_t i) {
      for (const pqxx::row& row : page) {
        json item = utils::MakeItemFromRow(row);
        items["items"][i++] = std::move(item);
      }
    }, i));
    i += NTHREADS;
  }

  for (auto& f : futures)
    f.get();

  return items;
}

static json GetHistoryBetween(DbManager& dbm, const std::string& id,
  const std::string& date1, const std::string& date2) {
    json items;
    pqxx::nontransaction nt = dbm.NonTransaction();
    const std::string format = "YYYY-MM-DDXHH24:MI:SSX";
    pqxx::result res = nt.exec("SELECT * FROM history WHERE "
      "id = '" + id + "' "
      "AND date::timestamp >= to_timestamp('" + date1 + "', '" + format + "') "
      "AND date::timestamp < to_timestamp('" + date2 + "', '" + format + "') "
      "ORDER BY date::timestamp, id;");
    items["items"] = json::value_t::array;
    items["items"].get_ptr<json::array_t*>()->reserve(res.size());
    std::vector<std::future<void>> futures;
    size_t i = 0;
    for (auto page : Paginate(res, NTHREADS)) {
      futures.push_back(std::async([&, page](size_t i) {
        for (const pqxx::row& row : page) {
          json item = utils::MakeItemFromRow(row);
          items["items"][i++] = std::move(item);
        }
      }, i));
      i += NTHREADS;
    }

    for (auto& f : futures)
      f.get();

    return items;
  }

}