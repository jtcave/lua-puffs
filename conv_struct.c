// conv_struct.c - convert Lua tables to/from puffs structs

#include <lauxlib.h>
#include <sys/namei.h>

#include "luapuffs.h"


// convert struct puffs_cn to a table
int luapuffs_pcn_push(lua_State *L, const struct puffs_cn *pcn)
{
  lua_createtable(L, 0, 4);
  
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
