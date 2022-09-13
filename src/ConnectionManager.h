#pragma once

#include <set>
#include "Connection.h"

class ConnectionManager {
public:
  ConnectionManager();

  void Start(connection_ptr c);

  void Stop(connection_ptr c);

  void StopAll();

private:
  std::set<connection_ptr> connections_;
};