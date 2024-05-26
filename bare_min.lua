#!/usr/bin/lua
-- bare minimum smoke test

-- kludge for quick testing
package.cpath = "./?.so"

puffs = require 'puffs'

local fsname = "barefs"
local mountpoint = "/mnt"
local pflags = 0 -- puffs.PUFFS_KFLAG_NOCACHE
local mflags = puffs.MNT_RDONLY | puffs.MNT_NOEXEC | puffs.MNT_NODEV

local ops = {}
function ops:lookup(dirnode, query)
   print("lookup got to lua!!!")
   print("dirnode", dirnode)
   print("query", query)
   print("dirnode:getcookie()", dirnode:getcookie())
   print("query.nameiop", query.nameiop)
   print("query.islast",    query.islast)
   print("query.flags",    query.flags)
   print("query.name",    query.name)
   print()
   
   if query.nameiop == puffs.NAMEI_LOOKUP then
      return nil, puffs.ENOENT
   else
      return nil, puffs.EACCES
   end
end

function main()
   local um = puffs.init(ops, fsname, pflags, fsname)
   local root_node = um:mount(mountpoint, mflags)
   --print("ops:", ops)
   --ud, hasud = debug.getuservalue(um, 1)
   --print("ud:", ud)
   --print("hasud:", hasud)
   print("entering main loop!")
   um:mainloop()
   print("bye!")
end

main() 
