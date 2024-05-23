#ifndef _LUAPUFFS_H
#define _LUAPUFFS_H

#include <lua.h>
#include <puffs.h>

/// metatable names ///

#define LUAPUFFS_MT_USERMOUNT "luapuffs_usermount"
 
/// globally available functions ///

int luapuffs_install_constants(lua_State *L);
void luapuffs__mkpops(lua_State *L, struct puffs_ops *pops);

/// internal structs ///

// Information needed to access the usermount object
struct luapuffs_ref_usermount {
  lua_State *L0;    // state for main coroutine
  int ref;          // reference in registry
};

/// userdata structs ///

// Wrap the usermount.
// This can't be a light userdata because it needs to have a metatable.
typedef struct luapuffs_ud_usermount {
  struct puffs_usermount *pu;
} luapuffs_ud_usermount;
#define LUAPUFFS_UV_USERMOUNT_OPS 1


#endif
