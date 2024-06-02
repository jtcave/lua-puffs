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

  // here's a macro to make things less unreadable
#define GET_INTEGER(dest, src)	     \
  ltype = lua_getfield(L, -1, #src); \
  if (ltype != LUA_TNIL) {	     \
    dest = lua_tointeger(L, -1);     \
  }				     \
  lua_pop(L, 1);
  // end #define
  
  // TODO: the rest of these
  // hellofs uses:
  // type mode nlink uid gid size bytes

  GET_INTEGER(vap->va_type, type);
  GET_INTEGER(vap->va_mode, mode);
  GET_INTEGER(vap->va_nlink, nlink);
  GET_INTEGER(vap->va_uid, uid);
  GET_INTEGER(vap->va_gid, gid);
  
  // TODO: fsid fileid

  GET_INTEGER(vap->va_size, size);

  // TODO: blocksize atime mtime ctime birthtime gen flags rdev

  GET_INTEGER(vap->va_bytes, bytes);

  // TODO: filerev vaflags
  return 0;
}

#undef GET_INTEGER

// unpack a single dirent
int luapuffs_dirent_pop(lua_State *L, ino_t *fileno, uint8_t *dtype, const char **name)
{
  int ltype;
  ltype = lua_getfield(L, -1, "fileno");
  if (ltype != LUA_TNIL) {
    *fileno = lua_tointeger(L, -1);
  }
  else {
    lua_pushstring(L, "missing fileno field in dirent");
    return 1;
  }
  lua_pop(L, 1);
    
  ltype = lua_getfield(L, -1, "dtype");
  if (ltype != LUA_TNIL) {
    *dtype = lua_tointeger(L, -1);
  }
  else {
    lua_pushstring(L, "missing dtype field in dirent");
    return 1;
  }
  lua_pop(L, 1);

  ltype = lua_getfield(L, -1, "name");
  if (ltype != LUA_TNIL) {
    *name = lua_tostring(L, -1);
  }
  else {
    lua_pushstring(L, "missing name field in dirent");
    return 1;
  }
  lua_pop(L, 2); // also pop dirent table as advertised
  return 0;
}

// convert the dirent list
// Lua args: return values of the readdir callback in direct order
int luapuffs_dirent_list_pop(lua_State *L, int nresults, struct dirent **dent,
			     off_t *readoff, size_t *reslen, int *eofflag)
{
  int ltype;
  ino_t fileno;
  uint8_t dtype;
  const char *name;
  // TODO: next_offset entry for file handles
  
  // check if first return is a table
  ltype = lua_type(L, 1);
  if (ltype != LUA_TTABLE) {
    lua_pushfstring(L, "expected table of dirent tables, got a %s", lua_typename(L, ltype));
    return 1;
  }

  // if the second return is false or absent, set eofflag
  if (!(nresults > 1 && lua_toboolean(L, 2))) {
    *eofflag = 1;
  }
  
  // walk dirent table
  lua_pushnil(L);
  while (lua_next(L, 1) != 0) {
    // unpack the dirent
    if (luapuffs_dirent_pop(L, &fileno, &dtype, &name) != 0) {
      // broken; relay the error
      return 1;
    }
    // feed it to puffs
    if (puffs_nextdent(dent, name, fileno, dtype, reslen) == 0) {
      // no room, so choke and stop the walk.
      // if this is a one-too-many error, there's a good chance the server can survive if
      // we let it
      lua_warning(L, "puffs_nextdent is full; did readdir return too many entries?", 0);
      break;
    }
    (*readoff)++;
  }

  // pop as advertised
  lua_settop(L, 0);
  
  return 0;


}
