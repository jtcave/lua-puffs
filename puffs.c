/* puffs.c
 * Lua wrapper for NetBSD puffs
 *
 * <inasert BSD license here> */

#include <puffs.h>
#include <lua.h>
#include <lauxlib.h>
#include <stdio.h> // for sscanf
#include <errno.h>
#include <stdlib.h>

#include "luapuffs.h"

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


// init, the usermount constructor
int luapuffs_init(lua_State *L)
{
  struct puffs_ops *pops;
  struct luapuffs_ref_usermount *ref_um;
  const char *fsname;
  const char *devname;
  int flags, ud_ref;

  // load args
  luaL_checktype(L, 1, LUA_TTABLE);
  fsname = luaL_checkstring(L, 2);
  flags = luaL_checkinteger(L, 3) | PUFFS_FLAG_PNCOOKIE;
  devname = lua_tostring(L, 4);
  if (devname == NULL) {
    devname = fsname; // shouldn't this be _PATH_PUFFS
  }
  
  // allocate object and wire up Lua side
  luapuffs_ud_usermount *um = lua_newuserdatauv(L, sizeof(luapuffs_ud_usermount), 1);
  luaL_setmetatable(L, LUAPUFFS_MT_USERMOUNT);
  lua_pushvalue(L, 1);  // ops table
  int hasuv = lua_setiuservalue(L, -2, LUAPUFFS_UV_USERMOUNT_OPS);
  if (hasuv == 0) {
    return luaL_error(L, "doesn't have the uservalue");
  }

  // install in registry and capture reference
  ref_um = malloc(sizeof(struct luapuffs_ref_usermount));
  ref_um->L0 = L;
  lua_pushvalue(L, -1);
  ud_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  ref_um->ref = ud_ref;
  
  // wire up the C ops table
  pops = malloc(sizeof(struct puffs_ops)); // will be freed by puffs_init

  // init
  PUFFSOP_INIT(pops);

  // fill vfs ops with no-ops
  PUFFSOP_SETFSNOP(pops, statvfs);
  PUFFSOP_SETFSNOP(pops, sync);
  PUFFSOP_SETFSNOP(pops, unmount);
  
  luapuffs__mkpops(L, pops);

  // initialize underlying library
  um->pu = puffs_init(pops, devname, fsname, ref_um, flags);
  return 1;
} 

// usermount methods //
int luapuffs_usermount_mount(lua_State *L)
{
  luapuffs_ud_usermount *ud_um = luaL_checkudata(L, 1, LUAPUFFS_MT_USERMOUNT);
  const char *dirpath = luaL_checkstring(L, 2);
  int mflags = luaL_checkinteger(L, 3);

  // TODO: encapsulate node in a Lua object (does this need use of the priv arg?)
  struct puffs_node *root_pn = puffs_pn_new(ud_um->pu, NULL);
  int mounterr = puffs_mount(ud_um->pu, dirpath, mflags, root_pn);
  if (mounterr != 0) {
    return luaL_error(L, "puffs_mount returned errno = %d", mounterr);
  }
  else {
    // TODO: encapsulate the node
    lua_pushinteger(L, (lua_Integer)root_pn);
    return 1;
  }
}

int luapuffs_usermount_mainloop(lua_State *L)
{
  luapuffs_ud_usermount *ud_um = luaL_checkudata(L, 1, LUAPUFFS_MT_USERMOUNT);

  // make it so ctrl-c unmounts the server
  puffs_unmountonsignal(SIGINT, 0);
  int result = puffs_mainloop(ud_um->pu);
  lua_pushboolean(L, result == 0);
  return 1;
}

static const luaL_Reg luapuffs_functions[] = {
  {"init", luapuffs_init},
  {"octal", luapuffs_octal},
  {NULL, NULL}
};

static const luaL_Reg luapuffs_usermount_methods[] = {
  {"mount", luapuffs_usermount_mount},
  {"mainloop", luapuffs_usermount_mainloop},
  {NULL, NULL},
};

LUALIB_API int luaopen_puffs(lua_State *L)
{

  // usermount metatable
  luaL_newmetatable(L, LUAPUFFS_MT_USERMOUNT);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, luapuffs_usermount_methods, 0);

  // module table
  lua_newtable(L);
  luaL_setfuncs(L, luapuffs_functions, 0);
  luapuffs_install_constants(L);
  return 1;
}
