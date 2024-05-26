// node.c - methods for node objects

#include "luapuffs.h"
#include <lauxlib.h>

int luapuffs_node_getcookie(lua_State *L);

static const luaL_Reg luapuffs_node_methods[] = {
  {"getcookie", luapuffs_node_getcookie},
  {NULL, NULL}
};

int luapuffs_node_getcookie(lua_State *L)
{
  //luapuffs_ud_node *ud_pn = luaL_checkudata(L, 1, LUAPUFFS_MT_NODE);
  lua_pushinteger(L, -1);
  return 1;
}

int luapuffs_node_push(lua_State *L, struct puffs_node *pn)
{
  luapuffs_ud_node *ud_pn = lua_newuserdatauv(L, sizeof(luapuffs_ud_usermount), 0);
  luaL_setmetatable(L, LUAPUFFS_MT_NODE);
  ud_pn->pn = pn;
  return 1;
}

void luapuffs_node_makemetatable(lua_State *L)
{
  luaL_newmetatable(L, LUAPUFFS_MT_NODE);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, luapuffs_node_methods, 0);
}
