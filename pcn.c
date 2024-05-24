// pcn.c - wrap puffs path components for lua //

#include <lauxlib.h>
#include <sys/namei.h>

#include "luapuffs.h"


int luapuffs_pcn_push(lua_State *L, const struct puffs_cn *pcn)
{
  /*luapuffs_ud_pcn *ud_pcn = lua_newuserdatauv(L, sizeof(luapuffs_ud_pcn), 0);
  luaL_setmetatable(L, LUAPUFFS_MT_PCN);
  ud_pcn->pcn = pcn;*/
  lua_createtable(L, 0, 3);
  
  lua_pushstring(L, "nameiop");
  lua_pushinteger(L, pcn->pcn_nameiop);
  lua_rawset(L, -3);
  
  lua_pushstring(L, "islast");
  lua_pushboolean(L, (pcn->pcn_flags & NAMEI_ISLASTCN) != 0);
  lua_rawset(L, -3);

  lua_pushstring(L, "flags");
  lua_pushinteger(L, pcn->pcn_flags);
  lua_rawset(L, -3);

  lua_pushstring(L, "name");
  lua_pushstring(L, pcn->pcn_name);
  lua_rawset(L, -3);

  return 1;
}

