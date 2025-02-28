#pragma once
#include "lua.hpp"
#include "misc_functions.hpp"

#include "LuaBoolean.hpp"
#include "LuaUserdata.hpp"
#include "LuaNil.hpp"

#include <variant>
#include <string>
#include <array>
#include <unordered_map>
#include <memory>

enum class LuaMultiValue : size_t { Number = 0, Function, String, Boolean, Table, Userdata, Nil };
inline const std::array<std::string, 7> LuaMultiValue_s = { "Number", "Function", "String", "Boolean", "Table", "Userdata", "Nil" };

class LuaTable final {
private:
    std::unordered_map<std::string_view, std::variant<lua_Number, lua_CFunction, std::string, LuaBoolean, std::shared_ptr<LuaTable>, LuaUserdata, LuaNil>> elements;
    mutable size_t counter_of_get = 0;

    void check_errors(LuaMultiValue expected_type, std::string_view key) const {
        if (elements.find(key) == elements.end())
            throw_error("[Table assert] Out of range, key = '" + std::string(key) + "'.");

        if (elements.at(key).index() != static_cast<size_t>(expected_type))
            throw_error("[Table] Bad argument #" + std::string(key) + " (" + LuaMultiValue_s[static_cast<size_t>(expected_type)] + " expected, got " + LuaMultiValue_s[elements.at(key).index()] + ")");
    }

public:
    LuaTable() = default;
    LuaTable(lua_State* lua_state, int index);
    LuaTable(const LuaTable& other);
    LuaTable(LuaTable&& tmp) noexcept;
    ~LuaTable() = default;
    LuaTable& operator=(const LuaTable& other);
    LuaTable& operator=(LuaTable&& tmp) noexcept;

    size_t size() const;
    size_t empty() const;
    LuaMultiValue get_type(std::string_view key) const;

    template<typename T>
    T get(std::string_view key) const {
        (void) key;
        throw_error("[Table assert] Unknown type for get<T>, mb you meant get<LuaUserdata, T>?");
    }

    template<>
    short get<short>(std::string_view key) const {
        check_errors(LuaMultiValue::Number, key);

        return static_cast<short>(std::get<lua_Number>(elements.at(key)));
    }

    template<>
    ushort_t get<ushort_t>(std::string_view key) const {
        check_errors(LuaMultiValue::Number, key);

        return static_cast<ushort_t>(std::get<lua_Number>(elements.at(key)));
    }

    template<>
    int get<int>(std::string_view key) const {
        check_errors(LuaMultiValue::Number, key);

        return static_cast<int>(std::get<lua_Number>(elements.at(key)));
    }

    template<>
    size_t get<size_t>(std::string_view key) const {
        check_errors(LuaMultiValue::Number, key);

        const lua_Number tmp = std::get<lua_Number>(elements.at(key));
        return static_cast<size_t>((tmp > 0) ? tmp : 0);
    }

    template<>
    double get<double>(std::string_view key) const {
        check_errors(LuaMultiValue::Number, key);

        return std::get<lua_Number>(elements.at(key));
    }

    template<>
    float get<float>(std::string_view key) const {
        check_errors(LuaMultiValue::Number, key);

        return static_cast<float>(std::get<lua_Number>(elements.at(key)));
    }

    template<>
    char get<char>(std::string_view key) const {
        check_errors(LuaMultiValue::String, key);

        const std::string_view tmp{std::get<std::string>(elements.at(key)).c_str(), std::get<std::string>(elements.at(key)).size()};
        return *(tmp.data());
    }

    template<>
    std::string get<std::string>(std::string_view key) const {
        check_errors(LuaMultiValue::String, key);

        return std::get<std::string>(elements.at(key));
    }

    template<>
    bool get<bool>(std::string_view key) const {
        check_errors(LuaMultiValue::Boolean, key);

        return static_cast<bool>(std::get<LuaBoolean>(elements.at(key)));
    }

    template<>
    LuaNil get<LuaNil>(std::string_view key) const {
        check_errors(LuaMultiValue::Nil, key);

        return std::get<LuaNil>(elements.at(key));
    }

    template<>
    LuaTable get<LuaTable>(std::string_view key) const {
        check_errors(LuaMultiValue::Table, key);

        return *(std::get<std::shared_ptr<LuaTable>>(elements.at(key)));
    }

    template<typename T>
    T get() const {
        return get<T>(std::to_string(1 + elements.size() - counter_of_get--));
    }

    template<typename T, typename APIStruct>
    APIStruct get(std::string_view key) const {
        if (!std::is_same_v<T, LuaUserdata>)
            throw_error("[Table assert] Unknown type for get<LuaUserdata, APIStruct>!");

        check_errors(LuaMultiValue::Userdata, key);

        return *(*static_cast<APIStruct**>(std::get<LuaUserdata>(elements.at(key))));
    }

    template<typename T, typename APIStruct>
    APIStruct get() const {
        return get<T, APIStruct>(std::to_string(1 + elements.size() - counter_of_get--));
    }
};

using LuaMultiValue_t = std::variant<lua_Number, lua_CFunction, std::string, LuaBoolean, LuaTable, LuaUserdata, LuaNil>;