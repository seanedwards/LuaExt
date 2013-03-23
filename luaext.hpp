#ifndef __LUAEXT_H__
#define __LUAEXT_H__

#include <lua.hpp>
#include <stdexcept>
#include <sstream>
#include <boost/lexical_cast.hpp>

inline void luaX_pushstdstring(lua_State* L, const std::string& str) {
	lua_pushlstring(L, str.c_str(), str.length());
}

inline void lua_pushstring(lua_State* L, const std::string& str) {
    luaX_pushstdstring(L, str);
}

inline std::string luaX_tostdstring(lua_State* L, int idx) {
	size_t len;
	const char* dat = lua_tolstring(L, idx, &len);
	return std::string(dat, len);
}

inline void luaX_push(lua_State* L, const std::string& s) { luaX_pushstdstring(L, s); }
inline void luaX_push(lua_State* L, const bool& b) { lua_pushboolean(L, b); }
inline void luaX_push(lua_State* L, const double& d) { lua_pushnumber(L, d); }
inline void luaX_push(lua_State* L, const float& d) { lua_pushnumber(L, d); }
inline void luaX_push(lua_State* L, const short int& d) { lua_pushinteger(L, d); }
inline void luaX_push(lua_State* L, const int& d) { lua_pushinteger(L, d); }
inline void luaX_push(lua_State* L, const long int& d) { lua_pushinteger(L, d); }
inline void luaX_push(lua_State* L, const char* s) { lua_pushstring(L, s); }


inline void luaX_to(lua_State* L, int i, std::string& s) { s = luaX_tostdstring(L, i); }
inline void luaX_to(lua_State* L, int i, bool& b) { b = (lua_toboolean(L, i) != false); }
inline void luaX_to(lua_State* L, int i, double& d) { d = (double)lua_tonumber(L, i); }
inline void luaX_to(lua_State* L, int i, float& d) { d = (float)lua_tonumber(L, i); }
inline void luaX_to(lua_State* L, int i, short int& d) { d = (short int)lua_tonumber(L, i); }
inline void luaX_to(lua_State* L, int i, int& d) { d = (int)lua_tonumber(L, i); }
inline void luaX_to(lua_State* L, int i, long int& d) { d = (long int)lua_tonumber(L, i); }
inline void luaX_to(lua_State* L, int i, const char* s) { s = lua_tostring(L, i); }

inline int _luaX_traceback(lua_State* L)
{
	std::string err = luaX_tostdstring(L, 1);
	std::stringstream errout;
	errout << err;
    
	int i = 0;
	lua_Debug dbg;
	while (lua_getstack(L, i++, &dbg)) {
		lua_getinfo(L, "nSl", &dbg);
		errout << std::endl << "\tin " << (dbg.name == NULL ? "`unknown`" : dbg.name) << ":" <<
        (dbg.currentline == -1 ? std::string("?") : boost::lexical_cast<std::string>(dbg.currentline)) <<
        " - " << dbg.short_src;
	}
	luaX_pushstdstring(L, errout.str());
	return 1;
}

inline void _luaX_tcall(lua_State* L)
{
	if (lua_pcall(L, 0, 0, -2))
	{
		std::string errmsg = luaX_tostdstring(L, -1);
		lua_pop(L, 2);
		throw std::runtime_error(errmsg.c_str());
	}
	lua_pop(L, 1);
}

template<typename R>
R _luaX_tcall(lua_State* L)
{
	if (lua_pcall(L, 0, 1, -2))
	{
		std::string errmsg = luaX_tostdstring(L, -1);
		lua_pop(L, 2);
		throw std::runtime_error(errmsg.c_str());
	}
    
	R ret;
	lua_to(L, -1, ret);
	lua_pop(L, 2);
	return ret;
}

template<typename V, typename... Args>
void _luaX_tcall(lua_State* L, const V& val, Args... args) {
    luaX_push(L, val);
    _luaX_tcall<V, Args...>(L, args...);
}

template<typename R, typename V, typename... Args>
R _luaX_tcall(lua_State* L, const V& val, Args... args) {
    luaX_push(L, val);
    return _luaX_tcall<R, Args...>(L, args...);
}

template<typename... Args>
void luaX_tcall(lua_State* L, Args... args)
{
	lua_pushcfunction(L, _luaX_traceback);
	lua_insert(L, -2);
    
    _luaX_tcall<Args...>(L, args...);
}

template<typename R, typename... Args>
R luaX_tcall(lua_State* L, Args... args)
{
	lua_pushcfunction(L, _luaX_traceback);
	lua_insert(L, -2);
    
    return _luaX_tcall<R, Args...>(L, args...);
}

#endif // __LUAEXT_H__
