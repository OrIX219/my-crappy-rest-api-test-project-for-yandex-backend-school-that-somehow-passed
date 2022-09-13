#include "ConnectionManager.h"

ConnectionManager::ConnectionManager() {}

void ConnectionManager::Start(connection_ptr c) {
  connections_.insert(c);
  c->Start();
}

void ConnectionManager::Stop(connection_ptr c) {
  connections_.erase(c);
  c->Stop();
}

void ConnectionManager::StopAll() {
  for (auto c: connections_)
    c->Stop();
  connections_.clear();
}