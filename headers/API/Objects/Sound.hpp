#pragma once
#include "lua_helper.hpp"

#include <SFML/Audio.hpp>
#include <sstream>

namespace API
{
    class Sound final : public sf::Sound {
    private:
        sf::SoundBuffer buffer;
        std::string path;

    public:
        Sound(const LuaStack& args);
        
        const std::string get_path() const;
        void play(bool is_reset);

        static int push_to_lua(lua_State* L) {
            lhelper::new_class<Sound>(L);

            static auto destructor = [](lua_State* L) {
                delete lhelper::get_object<Sound>(L, "Sound", 1);
                return 0;
            };

            static auto get_path = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);

                lua_pushstring(L, self->get_path().c_str());
                return 1;
            };

            static auto get_volume = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);

                lua_pushinteger(L, self->getVolume());
                return 1;
            };

            static auto set_volume = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);
                int volume = luaL_checkinteger(L, 2);

                self->setVolume(volume);
                return 0;
            };

            static auto get_loop = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);

                lua_pushboolean(L, self->getLoop());
                return 1;
            };

            static auto set_loop = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);
                bool is_repeat = lua_toboolean(L, 2);

                self->setLoop(is_repeat);
                return 0;
            };

            static auto play = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);
                bool is_reset = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;

                self->play(is_reset);
                return 0;
            };

            static auto stop = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);

                self->stop();
                return 0;
            };

            static auto is_play = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);

                lua_pushboolean(L, self->getStatus() == sf::SoundSource::Status::Playing);
                return 1;
            };

            static auto copy = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);

                lhelper::push_object<Sound>(L, {
                    self->get_path(),
                    lua_Number(self->getVolume()),
                    LuaBoolean(self->getLoop())
                });

                return 1;
            };

            static auto index_get = [](lua_State* L) {
                if (lua_isstring(L, 2)) {
                    const std::string_view key = luaL_checkstring(L, 2);

                    if (key == "get_path") lua_pushcfunction(L, get_path);
                    else if (key == "get_volume") lua_pushcfunction(L, get_volume);
                    else if (key == "set_volume") lua_pushcfunction(L, set_volume);
                    else if (key == "get_loop") lua_pushcfunction(L, get_loop);
                    else if (key == "set_loop") lua_pushcfunction(L, set_loop);
                    else if (key == "play") lua_pushcfunction(L, play);
                    else if (key == "stop") lua_pushcfunction(L, stop);
                    else if (key == "is_play") lua_pushcfunction(L, is_play);
                    else if (key == "copy") lua_pushcfunction(L, copy);
                    else lua_pushnil(L);
                }
                else {
                    lua_pushnil(L);
                }

                return 1;
            };

            static auto to_string = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);

                std::stringstream result;
                result << "{ \"" << self->get_path() << "\", " << self->getVolume() << "%, " << std::boolalpha << self->getLoop() << " }";

                lua_pushstring(L, std::move(result).str().c_str());
                return 1;
            };

            static auto is_equal = [](lua_State* L) {
                const auto self = lhelper::get_object<Sound>(L, "Sound", 1);
                const auto target = lhelper::get_object<Sound>(L, "Sound", 2);

                lua_pushboolean(L, (
                    self->get_path() == target->get_path() &&
                    self->getVolume() == target->getVolume() &&
                    self->getLoop() == target->getLoop() &&
                    self->getStatus() == target->getStatus()
                ));
                
                return 1;
            };

            lhelper::set_methods(L, "Sound", {
                { "__gc", destructor },
                { "__index", index_get },
                { "__tostring", to_string },
                { "__eq", is_equal }
            });

            return 1;
        }

        static int reg(lua_State* L) {
            LuaStack tmp(L, "Sound:register");
            lua_remove(L, -static_cast<int>(tmp.size()));

            return push_to_lua(L);
        }
    };
}