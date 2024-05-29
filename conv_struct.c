// conv_struct.c - convert Lua tables to/from puffs structs

#include <lua.h>
#include <lauxlib.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <puffs.h>

#include "luapuffs.h"

puffs_cookie_t luapuffs_getcookie(lua_State *L, const char *key)
{
  puffs_cookie_t result;
  luapuffs_ud_node *ud_pn;
  lua_getfield(L, -1, key);
  ud_pn = luaL_checkudata(L, -1, LUAPUFFS_MT_USERMOUNT);
  result = (ud_pn != NULL) ? ud_pn->pn : NULL;
  lua_pop(L, 1);
  return result;
}

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

// convert newinfo tables to struct puffs_newinfo
// Lua arguments: top of stack is the newinfo table
int luapuffs_newinfo_pop(lua_State *L, struct puffs_newinfo *pni)
{
  int ltype;
  enum vtype vtype;
  voff_t size;
  dev_t rdev;
  //struct vattr va;
  // TODO: vattl, cnttl (these need va)

  // sanity check
  luaL_checktype(L, -1, LUA_TTABLE);

  // TODO: do we automatically create nodes or no?
  puffs_newinfo_setcookie(pni, luapuffs_getcookie(L, "node"));
  
  ltype = lua_getfield(L, -1, "vtype");
  if (ltype == LUA_TNIL) {
    lua_pushstring(L, "newinfo table missing field 'vtype'");
    return 1;
  }
  vtype = lua_tointeger(L, -1);
  lua_pop(L, 1);
  puffs_newinfo_setvtype(pni, vtype);

  if (vtype == VREG) {
    ltype = lua_getfield(L, -1, "size");
    if (ltype == LUA_TNIL) {
      lua_pushstring(L, "newinfo table missing field 'size'");
      return 1;
    }
    size = lua_tointeger(L, -1);
    lua_pop(L, 1);
    puffs_newinfo_setsize(pni, size);
  }

  if (vtype == VBLK || vtype == VCHR) {
    ltype = lua_getfield(L, -1, "rdev");
    if (ltype == LUA_TNIL) {
      lua_pushstring(L, "newinfo table missing field 'rdev'");
      return 1;
    }
    rdev = lua_tointeger(L, -1);
    lua_pop(L, 1);
    puffs_newinfo_setrdev(pni, rdev);
  }

  // pop the table, as advertised
  lua_pop(L, 1);
  return 0;
}

// convert vattr tables to struct vattr
// Lua arguments: top of stack is the vattr table
int luapuffs_vattr_pop(lua_State *L, struct vattr *vap)
{
  luaL_checktype(L, -1, LUA_TTABLE);
  return 1;
}
