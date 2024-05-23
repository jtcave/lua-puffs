// node.c - methods for node objects

#include "luapuffs.h"
#include <lauxlib.h>

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
  lua_pop(L, 1);
}
