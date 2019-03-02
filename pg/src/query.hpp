#ifndef _QUERY_HPP
#define _QUERY_HPP

#include "interfaces.h"
#include "query_manager.hpp"

using namespace GarrysMod::Lua;

class DatabaseQuery : public LuaEventEmitter<239, DatabaseQuery> {
private:
  std::string query_string = "";
public:
  pqxx::connection* _connection;
  QueryManager* manager;
  
  DatabaseQuery(std::string query_string) : LuaEventEmitter() {
    this->query_string = query_string;
    AddMethod("run", run);
    AddMethod("run_sync", run_sync);
  }

  LUA_METHOD(run) {
    CHECK_CONNECTION

    LUA->ThrowError("Async execution not implemented yet :/");
    return 0;
  }

  LUA_METHOD(run_sync) {
    CHECK_CONNECTION

    auto fut = obj->manager->query(obj->query_string);

    try {
        auto ret = fut.get();

        LUA->PushBool(true);
        LuaValue(ret).PushTable(state);
        return 2;
    } catch(const std::exception& e) {
        LUA->PushBool(false);
        LUA->PushString(e.what());
        return 2;
    }
  }
};

#endif