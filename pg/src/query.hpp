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

    std::thread([](auto t, auto fut) {
      auto ret = fut.get();
      if (ret.index() == 0) {
        t->Emit("success", std::get<LuaValue::table_t>(ret));
      } else {
        t->Emit("error", std::get<std::string>(ret));
      }
    }, obj, obj->manager->query(obj->query_string)).detach();

    return 0;
  }

  LUA_METHOD(run_sync) {
    CHECK_CONNECTION

    auto fut = obj->manager->query(obj->query_string);

    auto ret = fut.get();
    if (ret.index() == 0) {
      LUA->PushBool(true);
      LuaValue(std::get<LuaValue::table_t>(ret)).PushTable(state);
      return 2;
    } else {
      LUA->PushBool(false);
      LUA->PushString(std::get<std::string>(ret).c_str());
      return 2;
    }
  }
};

#endif