// creds.c - puffs cred object

#include <lua.h>
#include <lauxlib.h>
#include <puffs.h>

#include "luapuffs.h"

int luapuffs_cred_getuid(lua_State *L);
int luapuffs_cred_getgid(lua_State *L);
int luapuffs_cred_getgroups(lua_State *L);
int luapuffs_cred_isuid(lua_State *L);
int luapuffs_cred_hasgroup(lua_State *L);
int luapuffs_cred_iskernel(lua_State *L);
int luapuffs_cred_isfs(lua_State *L);
int luapuffs_cred_isjuggernaut(lua_State *L);
int luapuffs_cred_access(lua_State *L);
int luapuffs_cred_access_chown(lua_State *L);
int luapuffs_cred_access_chmod(lua_State *L);
int luapuffs_cred_access_times(lua_State *L);

static const luaL_Reg luapuffs_cred_methods[] = {
  {"getuid", luapuffs_cred_getuid},
  {"getgid", luapuffs_cred_getgid},
  //{"getgroups", luapuffs_cred_getgroups},
  {"isuid", luapuffs_cred_isuid},
  //{"hasgroup", luapuffs_cred_hasgroup},
  {"iskernel", luapuffs_cred_iskernel},
  {"isfs", luapuffs_cred_isfs},
  {"isjuggernaut", luapuffs_cred_isjuggernaut},
  {"access", luapuffs_cred_access},
  //{"access_chown", luapuffs_cred_access_chown},
  //{"access_chmod", luapuffs_cred_access_chmod},
  //{"access_times", luapuffs_cred_access_times},
  {NULL, NULL}
};

const struct puffs_cred *unpack_cred(lua_State *L)
{
  luapuffs_ud_cred *ud_pcr = luaL_checkudata(L, 1, LUAPUFFS_MT_CRED);
  return ud_pcr->pcr;
}
  

int luapuffs_cred_getuid(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  uid_t uid;
  if (puffs_cred_getuid(pcr, &uid) == -1) {
    lua_pushnil(L);
  }
  else {
    lua_pushinteger(L, uid);
  }
  return 1;
}

int luapuffs_cred_getgid(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  gid_t gid;
  if (puffs_cred_getgid(pcr, &gid) == -1) {
    lua_pushnil(L);
  }
  else {
    lua_pushinteger(L, gid);
  }
  return 1;
}

int luapuffs_cred_getgroups(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  luaL_error(L, "unimplemented");
  return 0;
}

int luapuffs_cred_isuid(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  uid_t uid = lua_tointeger(L, 2);
  lua_pushboolean(L, puffs_cred_isuid(pcr, uid));
  return 1;
}

int luapuffs_cred_hasgroup(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  gid_t gid = lua_tointeger(L, 2);
  lua_pushboolean(L, puffs_cred_hasgroup(pcr, gid));
  return 1;
}

int luapuffs_cred_iskernel(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  lua_pushboolean(L, puffs_cred_iskernel(pcr));
  return 1;
}

int luapuffs_cred_isfs(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  lua_pushboolean(L, puffs_cred_isfs(pcr));  
  return 1;
}

int luapuffs_cred_isjuggernaut(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  lua_pushboolean(L, puffs_cred_isjuggernaut(pcr));
  return 1;
}


/* int
   puffs_access(enum vtype type, mode_t file_mode, uid_t owner, gid_t group,
                mode_t access_mode, const struct puffs_cred *pcr);
becomes
   cred:access(type, file_mode, owner, group, access_mode)*/
int luapuffs_cred_access(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  enum vtype type = lua_tointeger(L, 2);
  mode_t file_mode = lua_tointeger(L, 3);
  uid_t owner = lua_tointeger(L, 4);
  gid_t group = lua_tointeger(L, 5);
  mode_t access_mode = lua_tointeger(L, 6);

  lua_pushboolean(L, puffs_access(type, file_mode, owner, group, access_mode, pcr));
  return 1;
}

int luapuffs_cred_access_chown(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  luaL_error(L, "unimplemented");
  return 0;
}

int luapuffs_cred_access_chmod(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  luaL_error(L, "unimplemented");
  return 0;
}

int luapuffs_cred_access_times(lua_State *L)
{
  const struct puffs_cred *pcr = unpack_cred(L);
  luaL_error(L, "unimplemented");
  return 0;
}

int luapuffs_cred_push(lua_State *L, const struct puffs_cred *pcr)
{
  luapuffs_ud_cred *ud_pcr = lua_newuserdatauv(L, sizeof(luapuffs_ud_cred), 0);
  luaL_setmetatable(L, LUAPUFFS_MT_CRED);
  ud_pcr->pcr = pcr;
  return 1;
}

void luapuffs_cred_makemetatable(lua_State *L)
{
  luaL_newmetatable(L, LUAPUFFS_MT_CRED);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, luapuffs_cred_methods, 0);
}
