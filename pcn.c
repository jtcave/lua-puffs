// pcn.c - wrap puffs path components for lua //

#include <lauxlib.h>

#include "luapuffs.h"


int luapuffs_pcn_push(lua_State *L, const struct puffs_cn *pcn)
{
  luapuffs_ud_pcn *ud_pcn = lua_newuserdatauv(L, sizeof(luapuffs_ud_pcn), 0);
  luaL_setmetatable(L, LUAPUFFS_MT_PCN);
  ud_pcn->pcn = pcn;
  return 1;
}


void luapuffs_pcn_makemetatable(lua_State *L)
{
  luaL_newmetatable(L, LUAPUFFS_MT_PCN);
  lua_pop(L, 1);
}
