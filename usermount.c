// usermount.c - usermount object methods

#include <lua.h>
#include <lauxlib.h>
#include <err.h>

#include "luapuffs.h"

int luapuffs_usermount_mount(lua_State *L);
int luapuffs_usermount_mainloop(lua_State *L);

static const luaL_Reg luapuffs_usermount_methods[] = {
  {"mount", luapuffs_usermount_mount},
  {"mainloop", luapuffs_usermount_mainloop},
  {NULL, NULL},
};

// usermount methods //
int luapuffs_usermount_mount(lua_State *L)
{
  luapuffs_ud_usermount *ud_um = luaL_checkudata(L, 1, LUAPUFFS_MT_USERMOUNT);
  const char *dirpath = luaL_checkstring(L, 2);
  int mflags = luaL_checkinteger(L, 3);

  struct puffs_node *root_pn = puffs_pn_new(ud_um->pu, NULL);
  int mounterr = puffs_mount(ud_um->pu, dirpath, mflags, root_pn);
  if (mounterr != 0) {
    warn("puffs_mount");
    return luaL_error(L, "puffs_mount failed");
  }
  else {
    luapuffs_node_push(L, root_pn);
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

void luapuffs_usermount_makemetatable(lua_State *L)
{
  // usermount metatable
  luaL_newmetatable(L, LUAPUFFS_MT_USERMOUNT);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, luapuffs_usermount_methods, 0);
}
