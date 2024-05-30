#ifndef _LUAPUFFS_H
#define _LUAPUFFS_H

#include <lua.h>
#include <puffs.h>

/// metatable names ///

#define LUAPUFFS_MT_USERMOUNT "luapuffs_usermount"
#define LUAPUFFS_MT_NODE "luapuffs_node"
//#define LUAPUFFS_MT_PCN "luapuffs_pathcomp"
 
/// startup functions ///

int luapuffs_install_constants(lua_State *L);
void luapuffs_mkpops(lua_State *L, struct puffs_ops *pops);

/// internal structs ///

// Information needed to access the usermount object
struct luapuffs_ref_usermount {
  lua_State *L0;    // state for main coroutine
  int ref;          // reference in registry
};

/// userdata structs/functions ///

// Wrap the usermount.
// This can't be a light userdata because it needs to have a metatable.
typedef struct luapuffs_ud_usermount {
  struct puffs_usermount *pu;
} luapuffs_ud_usermount;
#define LUAPUFFS_UV_USERMOUNT_OPS 1

void luapuffs_usermount_makemetatable(lua_State *L);

// Wrap the node
typedef struct luapuffs_ud_node {
  struct puffs_node *pn;
} luapuffs_ud_node;

void luapuffs_node_makemetatable(lua_State *L);
int luapuffs_node_push(lua_State *L, struct puffs_node *pn);

/// struct<->table conversions ///

int luapuffs_pcn_push(lua_State *L, const struct puffs_cn *pcn);
int luapuffs_newinfo_pop(lua_State *L, struct puffs_newinfo *pni);
// int luapuffs_vattr_pop(lua_State *L, struct vattr *vap);


/// utilities ///

void luapuffs_stackdump(lua_State *L);

#endif
