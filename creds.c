// creds.c - puffs cred object

#include <lua.h>
#include <lauxlib.h>

#include "luapuffs.h"

// TODO: actual cred objects
int luapuffs_cred_push(lua_State *L, const struct puffs_cred *pcr)
{
  lua_pushstring(L, "DUMMY FOR CRED OBJECT");
  return 1;
}
