#include <iostream>
#include "lua.hpp"

int main()
{
	lua_State* L = luaL_newstate();

	luaL_openlibs(L);

	if (0 != luaL_dofile(L, "main.lua"))
	{
		std::cout << lua_tostring(L, -1) << std::endl;
	}

	system("pause");

	return 0;
}