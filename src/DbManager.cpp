#include "DbManager.h"

DbManager::DbManager(const std::string& db_name,
  const std::string& user,
  const std::string& addr,
  const std::string& port)
  : connection_("dbname = " + db_name
    + " user = " + user
    + " hostaddr = " + addr
    + " port = " + port) {}

pqxx::work DbManager::Work() {
  return pqxx::work(connection_);
}

pqxx::nontransaction DbManager::NonTransaction() {
  return pqxx::nontransaction(connection_);
}