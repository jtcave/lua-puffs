// op_shims.c - C-to-Lua thunks for puffs oeprations

#include <lua.h>
#include <lauxlib.h>
#include <puffs.h>
#include <errno.h>
#include <stdlib.h>
#include <err.h>

#include "luapuffs.h"

#define PUFFS_CALLBACK int

PUFFSOP_PROTOS(luapuffs_shim);

// Because puffs uses token-pasting macros to install callbacks, we can't do it in a
// table-driven way or use a helper function.
// So the best approach is to fight macro with macro.
#define TRY_INSTALL_POP(fsornode, opname)				\
  lua_pushstring(L, #opname);						\
  lookup_type = lua_gettable(L, 1);					\
  if (lookup_type != LUA_TNIL) {					\
    if (lookup_type != LUA_TFUNCTION) {					\
      luaL_error(L, "entry for " #opname " must be a function");	\
    }									\
    PUFFSOP_SET(pops, luapuffs_shim, fsornode, opname);			\
  }									\
  lua_pop(L, 1);
// end #define



// Wire up the ops table to the usermount
// Lua args: bottom of stack = ops table, top of stack = usermount object
void luapuffs_mkpops(lua_State *L, struct puffs_ops *pops)
{
  int lookup_type;
  // sanity check
  luaL_checkudata(L, -1, LUAPUFFS_MT_USERMOUNT);
  
  // init
  //PUFFSOP_INIT(pops);

  // initialize vfs ops with no-ops
  PUFFSOP_SETFSNOP(pops, statvfs);
  PUFFSOP_SETFSNOP(pops, sync);
  PUFFSOP_SETFSNOP(pops, unmount);

  // demand a lookup method in ops table
  lua_pushstring(L, "lookup");
  lookup_type = lua_gettable(L, 1);
  if (lookup_type == LUA_TNIL) {
    luaL_error(L, "lookup operation is mandatory");
  }
  else if (lookup_type != LUA_TFUNCTION) {
    luaL_error(L, "operation passed as lookup isn't a function");
  }
  lua_pop(L, 1);

  // wire up lookup
  PUFFSOP_SET(pops, luapuffs_shim, node, lookup);

  // install other operations if present
  TRY_INSTALL_POP(fs, unmount);

  // sanity check
  luaL_checkudata(L, -1, LUAPUFFS_MT_USERMOUNT);
}

// Called when the coroutine throws an error
// TODO: let user toggle between just displaying the message and insta-death
int luapuffs_shim_onerror(lua_State *L0, lua_State *L1)
{
  // extract causal error message
  const char *message = lua_tostring(L1, -1);
  // build traceback and throw on main thread
  luaL_traceback(L0, L1, message, 1);
  return lua_error(L0);
}

// Handle yields from a coroutine
// TODO: dispatch to one of the framebuf yields as requested
int luapuffs_shim_onyield(struct puffs_usermount *pu)
{
  struct puffs_cc *pcc = puffs_cc_getcc(pu);
  if (pcc == NULL) {
    return 1;
  }
  else {
    puffs_cc_yield(pcc);
    return 0;
  }
}


/// here come the shims ///

// Not every shim is implemented yet, sadly.
// If you're looking for stubs, they use this macro:
//#define EMPTY_STUB _Pragma ("GCC warning \"function is a stub \""  ); return ENOSYS;
#define EMPTY_STUB return ENOSYS;

// suppress this warning because it'll trip several times per stub
#pragma GCC diagnostic ignored "-Wunused-parameter"

// To cut down on repetition, we use prolog, coroutine entry, and epilog macros

#define SHIM_PROLOG(name)						\
  int nresults, coro_status, oldtop, retval=0;				\
  struct luapuffs_ref_usermount *ud_ref;				\
  lua_State *L0, *L1;							\
  /* get Lua state and usermount object */				\
  ud_ref = puffs_getspecific(pu);					\
  L0 = ud_ref->L0;							\
  oldtop = lua_gettop(L0);						\
  lua_rawgeti(L0, LUA_REGISTRYINDEX, ud_ref->ref);			\
  /* get callback */							\
  lua_getiuservalue(L0, -1, LUAPUFFS_UV_USERMOUNT_OPS);			\
  lua_pushstring(L0,  #name);						\
  lua_gettable(L0, -2);							\
  /* spin up new coroutine;  give it the callback and usermount */	\
  L1 = lua_newthread(L0);						\
  lua_xmove(L0, L1, 2);
// end #define

#define SHIM_ENTER_CORO(nargs)						\
  /* jump to here when the coroutine is resumed */			\
  coro_entry:								\
  /* enter coroutine */							\
  coro_status = lua_resume(L1, L0, nargs, &nresults);			\
  /* handle yield/error */						\
  if (coro_status == LUA_YIELD) {					\
    luapuffs_shim_onyield(pu);						\
    goto coro_entry;							\
  }									\
  else if (coro_status != LUA_OK) {					\
    retval = luapuffs_shim_onerror(L0, L1);				\
    goto epilog; /* in SHIM_EPILOG */					\
  }									\
  /* code to handle the return values follows */
// end #define

#define SHIM_EPILOG				\
  /* jump here from SHIM_ENTER_CORO */		\
  epilog:					\
  lua_settop(L0, oldtop);			\
  return retval;
// end #define

PUFFS_CALLBACK
luapuffs_shim_fs_statvfs(struct puffs_usermount *pu, struct puffs_statvfs *sbp)
{
  EMPTY_STUB;
}
  
PUFFS_CALLBACK
luapuffs_shim_fs_sync(struct puffs_usermount *pu, int waitfor,
		      const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_fs_fhtonode(struct puffs_usermount *pu, void *fid, size_t fidsize,
			  struct puffs_newinfo *pni)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_fs_nodetofh(struct puffs_usermount *pu, puffs_cookie_t cookie,
			  void *fid, size_t *fidsize)
{
  EMPTY_STUB;
}


PUFFS_CALLBACK
luapuffs_shim_fs_extattrctl(struct puffs_usermount *pu, int cmd,
			    puffs_cookie_t cookie, int flags, int attrnamespace,
			    const char *attrname)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_fs_unmount(struct puffs_usermount *pu, int flags)
{
  SHIM_PROLOG(unmount);
  
  lua_pushinteger(L1, flags);

  SHIM_ENTER_CORO(2);

  // return first result, or 0 if no results
  retval = (nresults != 0) ? lua_tointeger(L1, -nresults) : 0;
  
  SHIM_EPILOG;
}


PUFFS_CALLBACK
luapuffs_shim_node_lookup(struct puffs_usermount *pu, puffs_cookie_t opc,
			  struct puffs_newinfo *pni, const struct puffs_cn *pcn)
{
  SHIM_PROLOG(lookup);

  // push args
  luapuffs_node_push(L1, opc);
  luapuffs_pcn_push(L1, pcn);

  SHIM_ENTER_CORO(3);

  if (nresults == 0) {
    luaL_error(L0, "lookup callback returned nothing");
    return EPROTO;
  }
  
  lua_insert(L1, -nresults);
  if (lua_toboolean(L1, -1)) {
    // unpack the struct
    if (luapuffs_newinfo_pop(L1, pni)) {
      retval = 0;
    }
    else {
      // missing struct
      lua_xmove(L1, L0, 1);
      lua_error(L0);
      retval = EBADRPC;
    }
  }
  else {
    lua_pop(L1, 1);
    if (nresults == 1) {
      // they didn't give up an error code, we gotta pick one
      retval = EPROTO;
    }
    else {
      retval = lua_tointeger(L1, -nresults + 1);
    }
  }
  
  SHIM_EPILOG;
}


PUFFS_CALLBACK
luapuffs_shim_node_create(struct puffs_usermount *pu, puffs_cookie_t opc,
			  struct puffs_newinfo *pni, const struct puffs_cn *pcn,
			  const struct vattr *vap)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_mknod(struct puffs_usermount *pu, puffs_cookie_t opc,
			 struct puffs_newinfo *pni, const struct puffs_cn *pcn,
			 const struct vattr *vap)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_open(struct puffs_usermount *pu, puffs_cookie_t opc, int mode,
			const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_open2(struct puffs_usermount *pu, puffs_cookie_t opc,
			 int modep, const struct puffs_cred *pcr, int *oflags)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_close(struct puffs_usermount *pu, puffs_cookie_t opc,
			 int flags, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_access(struct puffs_usermount *pu, puffs_cookie_t opc,
			  int mode, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_getattr(struct puffs_usermount *pu, puffs_cookie_t opc,
			   struct vattr *vap, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_setattr(struct puffs_usermount *pu, puffs_cookie_t opc,
			   const struct vattr *vap, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_getattr_ttl(struct puffs_usermount *pu, puffs_cookie_t opc,
			       struct vattr *vap, const struct puffs_cred *pcr,
			       struct timespec *va_ttl)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_setattr_ttl(struct puffs_usermount *pu, puffs_cookie_t opc,
			       struct vattr *vap, const struct puffs_cred *pcr,
			       struct timespec *va_ttl, int xflag)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_pathconf(struct puffs_usermount *pu, puffs_cookie_t opc,
			    int name, __register_t *retval)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_advlock(struct puffs_usermount *pu, puffs_cookie_t opc,
			   void *id, int op, struct flock *fl, int flags)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_poll(struct puffs_usermount *pu, puffs_cookie_t opc,
			int *events)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_mmap(struct puffs_usermount *pu, puffs_cookie_t opc,
			int flags, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_fsync(struct puffs_usermount *pu, puffs_cookie_t opc,
			 const struct puffs_cred *pcr, int flags, off_t offlo, off_t offhi)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_seek(struct puffs_usermount *pu, puffs_cookie_t opc,
			off_t oldoff, off_t newoff, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_remove(struct puffs_usermount *pu, puffs_cookie_t opc,
			  puffs_cookie_t targ, const struct puffs_cn *pcn)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_link(struct puffs_usermount *pu, puffs_cookie_t opc,
			puffs_cookie_t targ, const struct puffs_cn *pcn)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_rename(struct puffs_usermount *pu, puffs_cookie_t opc,
			  puffs_cookie_t src, const struct puffs_cn *pcn_src,
			  puffs_cookie_t targ_dir, puffs_cookie_t targ,
			  const struct puffs_cn *pcn_targ)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_mkdir(struct puffs_usermount *pu, puffs_cookie_t opc,
			 struct puffs_newinfo *pni, const struct puffs_cn *pcn,
			 const struct vattr *vap)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_rmdir(struct puffs_usermount *pu, puffs_cookie_t opc,
			 puffs_cookie_t targ, const struct puffs_cn *pcn)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_readdir(struct puffs_usermount *pu, puffs_cookie_t opc,
			   struct dirent *dent, off_t *readoff, size_t *reslen,
			   const struct puffs_cred *pcr, int *eofflag, off_t *cookies,
			   size_t *ncookies)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_symlink(struct puffs_usermount *pu, puffs_cookie_t opc,
			   struct puffs_newinfo *pni, const struct puffs_cn *pcn_src,
			   const struct vattr *vap, const char *link_target)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_readlink(struct puffs_usermount *pu, puffs_cookie_t opc,
			    const struct puffs_cred *pcr, char *link, size_t *linklen)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_read(struct puffs_usermount *pu, puffs_cookie_t opc,
			uint8_t *buf, off_t offset, size_t *resid,
			const struct puffs_cred *pcr, int ioflag)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_write(struct puffs_usermount *pu, puffs_cookie_t opc,
			 uint8_t *buf, off_t offset, size_t *resid,
			 const struct puffs_cred *pcr, int ioflag)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_write2(struct puffs_usermount *pu, puffs_cookie_t opc,
			  uint8_t *buf, off_t offset, size_t *resid,
			  const struct puffs_cred *pcr, int ioflag, int xflag)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_abortop(struct puffs_usermount *pu, puffs_cookie_t opc,
			   const struct puffs_cn *pcn)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_getextattr(struct puffs_usermount *pu, puffs_cookie_t opc,
			      int attrnamespace, const char *attrname, size_t *attrsize,
			      uint8_t *attr, size_t *resid, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_setextattr(struct puffs_usermount *pu, puffs_cookie_t opc,
			      int attrnamespace, const char *attrname, uint8_t *attr,
			      size_t *resid, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_listextattr(struct puffs_usermount *pu, puffs_cookie_t opc,
			       int attrnamespace, size_t *attrssize, uint8_t *attrs, size_t *resid,
			       int flag, const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_deleteextattr(struct puffs_usermount *pu, puffs_cookie_t opc,
				 int attrnamespace, const char *attrname,
				 const struct puffs_cred *pcr)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_fallocate(struct puffs_usermount *pu, puffs_cookie_t opc,
			     off_t pos, off_t len)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_fdiscard(struct puffs_usermount *pu, puffs_cookie_t opc,
			    off_t pos, off_t len)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_print(struct puffs_usermount *pu, puffs_cookie_t opc)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_reclaim(struct puffs_usermount *pu, puffs_cookie_t opc)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_reclaim2(struct puffs_usermount *pu, puffs_cookie_t opc,
			    int nlookup)
{
  EMPTY_STUB;
}

PUFFS_CALLBACK
luapuffs_shim_node_inactive(struct puffs_usermount *pu, puffs_cookie_t opc)
{
  EMPTY_STUB;
}
