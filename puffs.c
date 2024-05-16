/* puffs.c
 * Lua wrapper for NetBSD puffs
 *
 * <inasert BSD license here> */

#include <puffs.h>
#include <lua.h>
#include <lauxlib.h>
#include <stdio.h> // for sscanf

#include "luapuffs.h"

int luapuffs_hello(lua_State *L)
{
  lua_pushliteral(L, "Hello from puffs");
  return 1;
}

int luapuffs_octal(lua_State *L)
{
  const char *numstr = lua_tolstring(L, 1, NULL);
  if (numstr != NULL) {
    unsigned int num_value = 0;
    sscanf(numstr, "%o", &num_value);
    lua_pushinteger(L, num_value);
  }
  else {
    lua_pushinteger(L, 0);
  }
  return 1;
}

static const luaL_Reg luapuffs_functions[] = {
  {"hello", luapuffs_hello},
  //{"init", luapuffs_init},
  {"octal", luapuffs_octal},
  {NULL, NULL}
};

LUALIB_API int luaopen_puffs(lua_State *L)
{
  //luaL_newlib(L, luapuffs_functions);
  lua_newtable(L);
  luaL_setfuncs(L, luapuffs_functions, 0);
  luapuffs_install_constants(L);
  return 1;
}
