#pragma once

#include "interfaces.h"

using namespace GarrysMod::Lua;

class QueryManager : public LuaEventEmitter<239, QueryManager> {
public:
    pqxx::connection* _connection = nullptr;
    char* type_map = nullptr;

    std::future<LuaValue::table_t> query(std::string sql) {
        std::promise<LuaValue::table_t> p;
        try {
            auto work = new pqxx::work(*this->_connection);
            pqxx::result res = work->exec(sql);
            work->commit();

            int size = res.size();
            int i = 1; // since Lua tables start at 1
            LuaValue::table_t res_t;

            if (size > 0) {
                for (auto row : res) {
                    LuaValue::table_t row_t;
                    for (auto col : row) {
                        char ts = this->type_map[col.type()];

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

            p.set_value(res_t);
        } catch (std::exception &e) {
            p.set_exception(std::current_exception());
        }
        return p.get_future();
    }
};