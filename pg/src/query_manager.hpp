#pragma once

#include "interfaces.h"
#include "ThreadPool.h"

using namespace GarrysMod::Lua;

class QueryManager : public LuaEventEmitter<239, QueryManager> {
private:
    std::unique_ptr<ThreadPool> pool;
public:
    QueryManager() {
        pool = std::make_unique<ThreadPool>(1);
    }

    pqxx::connection* _connection = nullptr;
    char* type_map = nullptr;

    std::future<std::variant<LuaValue::table_t, std::string>> query(std::string sql) {
        auto lambda = [](pqxx::connection* connection, char* type_map, std::string query) {
            try {
                auto work = new pqxx::work(*connection);
                pqxx::result res = work->exec(query);
                work->commit();

                int size = res.size();
                int i = 1; // since Lua tables start at 1
                LuaValue::table_t res_t;

                if (size > 0) {
                    for (auto row : res) {
                        LuaValue::table_t row_t;
                        for (auto col : row) {
                            char ts = type_map[col.type()];

                            if (col.is_null()) {
                                row_t.emplace(std::make_pair(col.name(), LuaValue::Make(Type::NIL)));
                            } else {
                                switch (ts) {
                                case TYPCATEGORY_BOOLEAN:
                                    row_t.emplace(std::make_pair(col.name(), col.as<bool>()));
                                    break;
                                case TYPCATEGORY_NUMERIC:
                                case TYPCATEGORY_ENUM:
                                    row_t.emplace(std::make_pair(col.name(), col.as<double>()));
                                    break;
                                case TYPCATEGORY_INVALID:
                                    row_t.emplace(std::make_pair(col.name(), LuaValue::Make(Type::NIL)));
                                    break;
                                default:
                                    row_t.emplace(std::make_pair(col.name(), col.c_str()));
                                }
                            }
                        }
                        res_t[i++] = row_t;
                    }
                }

                return std::variant<LuaValue::table_t, std::string> { res_t };
            } catch (std::exception &e) {
                return std::variant<LuaValue::table_t, std::string> { e.what() };
            }
        };
        return pool->enqueue(lambda, _connection, type_map, sql);
    }
};