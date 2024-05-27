// node.c - methods for node objects

#include "luapuffs.h"
#include <lauxlib.h>
#include <assert.h>

// We intern node objects so that, if the underlying puffs_node pointers are equal, the node
// objects are themselves identical.
#define NODE_CACHE "_node_cache"

int luapuffs_node_getcookie(lua_State *L);

static const luaL_Reg luapuffs_node_methods[] = {
  {"getcookie", luapuffs_node_getcookie},
  {NULL, NULL}
};

int luapuffs_node_getcookie(lua_State *L)
{
  luapuffs_ud_node *ud_pn = luaL_checkudata(L, 1, LUAPUFFS_MT_NODE);
  lua_pushinteger(L, (lua_Integer)(ud_pn->pn));
  return 1;
}

// retrieve a cached node object
// TODO: delete from cache when nodes are reclaimed
int luapuffs_node_fetch(lua_State *L, struct puffs_node *pn)
{
  // get the node cache
  luaL_getmetatable(L, LUAPUFFS_MT_NODE);
  lua_getfield(L, -1, NODE_CACHE);
  lua_remove(L, -2); // drop metatable
  luaL_checktype(L, -1, LUA_TTABLE);  // sanity check


  // probe the cache entry
  lua_pushlightuserdata(L, pn);
  int cache_state = lua_gettable(L, -2);
  lua_remove(L, -2); // drop cache table
  if (cache_state == LUA_TNIL) {
    // cache miss
    lua_pop(L, 1);  // pop nil
    return 0;
  }
  else {
    // cache hit
    assert(luaL_checkudata(L, -1, LUAPUFFS_MT_NODE) != NULL);  // sanity check
    return 1;
  }
}

// store node at top-of-stack in cache
void luapuffs_node_store(lua_State *L, struct puffs_node *pn)
{
  // sanity check
  luaL_checkudata(L, -1, LUAPUFFS_MT_NODE);
  // get the node cache
  luaL_getmetatable(L, LUAPUFFS_MT_NODE);
  lua_getfield(L, -1, NODE_CACHE);
  lua_remove(L, -2); // drop metatable
  luaL_checktype(L, -1, LUA_TTABLE);  // sanity check

  // write to cache
  lua_pushlightuserdata(L, pn);
  lua_pushvalue(L, -3);  // val,cache,key -> cache,key,val
  luaL_checkudata(L, -1, LUAPUFFS_MT_NODE);  // sanity check
  lua_settable(L, -3);
  lua_pop(L, 1); // drop cache table
}

// box a puffs_node into a Lua node object
int luapuffs_node_push(lua_State *L, struct puffs_node *pn)
{
  int found = luapuffs_node_fetch(L, pn);
  if (found == 0) {
    // not in cache; create new node object
    luapuffs_ud_node *ud_pn = lua_newuserdatauv(L, sizeof(luapuffs_ud_usermount), 0);
    luaL_setmetatable(L, LUAPUFFS_MT_NODE);
    ud_pn->pn = pn;
    luapuffs_node_store(L, pn);
    return 1;
  }
  else {
    // node already on stack
    luaL_checkudata(L, -1, LUAPUFFS_MT_NODE); // sanity check
    return found;
  }
}

void luapuffs_node_makemetatable(lua_State *L)
{
  luaL_newmetatable(L, LUAPUFFS_MT_NODE);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, luapuffs_node_methods, 0);
  lua_newtable(L);
  lua_setfield(L, -2, NODE_CACHE);
}
