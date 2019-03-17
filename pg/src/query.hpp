#ifndef _QUERY_HPP
#define _QUERY_HPP

#include "interfaces.h"
#include "query_manager.hpp"

using namespace GarrysMod::Lua;

class DatabaseQuery : public LuaEventEmitter<239, DatabaseQuery> {
private:
  std::string query_string = "";
  bool debug = false;
public:
  pqxx::connection* _connection;
  QueryManager* manager;
  
  DatabaseQuery(std::string query_string) : LuaEventEmitter() {
    this->query_string = query_string;
    AddMethod("enable_debug", enable_debug);
    AddMethod("run", run);
    AddMethod("run_sync", run_sync);
  }

  LUA_METHOD(enable_debug) {
    CHECK_CONNECTION

    obj->debug = true;

    return 0;
  }

  LUA_METHOD(run) {
    CHECK_CONNECTION

    std::string query = obj->query_string;
    bool debug = obj->debug;

    std::thread([debug, query](auto t, auto fut) {
      if (debug) printf("[PGQuery] waiting for result of %s\n", query.c_str());
      auto ret = fut.get();
      if (debug) printf("[PGQuery] got result for %s: %d\n", query.c_str(), ret.index());
      if (ret.index() == 0) {
        t->Emit("success", std::get<LuaValue::table_t>(ret));
      } else {
        t->Emit("error", std::get<std::string>(ret));
      }
      if (debug) printf("[PGQuery] emitting result complete for %s\n", query.c_str());
    }, obj, obj->manager->query(query)).detach();

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