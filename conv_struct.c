// conv_struct.c - convert Lua tables to/from puffs structs

#include <lua.h>
#include <lauxlib.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <puffs.h>

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

// convert newinfo tables to struct puffs_newinfo
// Lua arguments: top of stack is the newinfo table
int luapuffs_newinfo_pop(lua_State *L, struct puffs_newinfo *pni)
{
  int ltype;
  luapuffs_ud_node *ud_pn;
  puffs_cookie_t pcn;
  enum vtype vtype;
  voff_t size;
  dev_t rdev;
  //struct vattr va;
  // TODO: vattl, cnttl (these need va)

  // sanity check
  ltype = lua_type(L, -1);
  if (ltype != LUA_TTABLE) {
    lua_pushfstring(L, "expected newinfo table, got a %s", lua_typename(L, ltype));
    return 1;
  }

  // TODO: do we automatically create nodes or no?
  ltype = lua_getfield(L, -1, "node");
  if (ltype == LUA_TNIL) {
    lua_pushstring(L, "newinfo table missing field 'node'");
    return 1;
  }
  ud_pn = luaL_checkudata(L, -1, LUAPUFFS_MT_NODE);
  pcn = ud_pn->pn;
  lua_pop(L, 1);
  puffs_newinfo_setcookie(pni, pcn);
  
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
  int ltype;
  ltype = lua_type(L, -1);
  if (ltype != LUA_TTABLE) {
    lua_pushfstring(L, "expected vattr table, got a %s", lua_typename(L, ltype));
    return 1;
  }

  // the user isn't expected to provide every field, so initialize the struct
  puffs_vattr_null(vap);
  
  // TODO: the rest of these
  // hellofs uses:
  // type mode nlink uid gid size bytes
  
  ltype = lua_getfield(L, -1, "type");
  if (ltype != LUA_TNIL) {
    vap->va_type = lua_tointeger(L, -1);
  }
  lua_pop(L, 1);

  ltype = lua_getfield(L, -1, "mode");
  if (ltype != LUA_TNIL) {
    vap->va_mode = lua_tointeger(L, -1);
  }
  lua_pop(L, 1);

  ltype = lua_getfield(L, -1, "nlink");
  if (ltype != LUA_TNIL) {
    vap->va_nlink = lua_tointeger(L, -1);
  }
  lua_pop(L, 1);
  
  ltype = lua_getfield(L, -1, "uid");
  if (ltype != LUA_TNIL) {
    vap->va_uid = lua_tointeger(L, -1);
  }
  lua_pop(L, 1);

  ltype = lua_getfield(L, -1, "gid");
  if (ltype != LUA_TNIL) {
    vap->va_gid = lua_tointeger(L, -1);
  }
  lua_pop(L, 1);

  // TODO: fsid fileid

  ltype = lua_getfield(L, -1, "size");
  if (ltype != LUA_TNIL) {
    vap->va_size = lua_tointeger(L, -1);
  }
  lua_pop(L, 1);

  // TODO: blocksize atime mtime ctime birthtime gen flags rdev

  ltype = lua_getfield(L, -1, "bytes");
  if (ltype != LUA_TNIL) {
    vap->va_bytes = lua_tointeger(L, -1);
  }
  lua_pop(L, 1);

  // TODO: filerev vaflags
  
  return 0;
}
