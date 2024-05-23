// op_shims.c - C-to-Lua thunks for puffs oeprations

#include <lua.h>
#include <lauxlib.h>
#include <puffs.h>
#include <errno.h>
#include <stdlib.h>

#include "luapuffs.h"

// prototypes (TODO: just use the macro)
int luapuffs_node_lookup(struct puffs_usermount *pu, puffs_cookie_t opc,
			 struct puffs_newinfo *pni, const struct puffs_cn *pcn);


// Wire up the ops table to the usermount
// Lua args: bottom of stack = ops table, top of stack = usermount object
void luapuffs__mkpops(lua_State *L, struct puffs_ops *pops)
{
  // sanity check
  luaL_checkudata(L, -1, LUAPUFFS_MT_USERMOUNT);
  
  // init
  //PUFFSOP_INIT(pops);

  // fill vfs ops with no-ops
  PUFFSOP_SETFSNOP(pops, statvfs);
  PUFFSOP_SETFSNOP(pops, sync);
  PUFFSOP_SETFSNOP(pops, unmount);

  // (reminder: ops table is arg 1, usermount is -1)
  
  // demand a lookup method in ops table
  lua_pushstring(L, "lookup");
  int lookup_type = lua_gettable(L, 1);
  if (lookup_type == LUA_TNIL) {
    luaL_error(L, "lookup operation is mandatory");
  }
  else if (lookup_type != LUA_TFUNCTION) {
    luaL_error(L, "operation passed as lookup isn't a function");
  }
  lua_pop(L, 1);

  // wire up lookup
  // TODO: everything else
  PUFFSOP_SET(pops, luapuffs, node, lookup);
}


int luapuffs_node_lookup(struct puffs_usermount *pu, puffs_cookie_t opc,
			  struct puffs_newinfo *pni, const struct puffs_cn *pcn)
{
  int nresults, coro_status;
  
  // get Lua state and object for pu
  struct luapuffs_ref_usermount *ud_ref = puffs_getspecific(pu);
  lua_State *L0 = ud_ref->L0;
  lua_rawgeti(L0, LUA_REGISTRYINDEX, ud_ref->ref);

  // get callback
  lua_getiuservalue(L0, -1, LUAPUFFS_UV_USERMOUNT_OPS);
  lua_pushstring(L0, "lookup");
  lua_gettable(L0, -2);

  // spin up new coroutine and push args
  lua_State *L1 = lua_newthread(L0);
  lua_xmove(L0, L1, 2);   // xfer callback, usermount
  lua_pushstring(L1, "DUMMY FOR DIRNODE/OPC");
  lua_pushstring(L1, "DUMMY FOR QUERY/PCN");
  coro_status = lua_resume(L1, L0, 3, &nresults);

  // TODO: process return values
  return ENOENT;
}
