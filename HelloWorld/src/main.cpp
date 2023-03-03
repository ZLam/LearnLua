#include <iostream>
#include "lua.hpp"
#include "helper_func.h"

void Lua_AddSearchPath(lua_State* L, const std::string& InPath)
{
	std::string AllPath;
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	auto s = lua_tostring(L, -1);
	std::cout << s << std::endl;
	AllPath.append(lua_tostring(L, -1));
	AllPath.append(";");
	AllPath.append(InPath.c_str());
	lua_pushstring(L, AllPath.c_str());
	lua_setfield(L, -3, "path");
	lua_pop(L, 2);
}

int main()
{
	lua_State* L = luaL_newstate();

	luaL_openlibs(L);

	if (0 != luaL_dofile(L, ScriptFullPath("main.lua").c_str()))
	{
		std::cout << lua_tostring(L, -1) << std::endl;
	}

	system("pause");

	return 0;
}
