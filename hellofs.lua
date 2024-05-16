-- quick protoype of a hello world FS

-- kludge for quick testing

package.cpath = "./?.so"
puffs = require 'puffs'

local fsname = "hellofs"
local mountpoint = "/mnt"
local pflags = puffs.PUFFS_KFLAG_NOCACHE
local mflags = puffs.MNT_RDONLY | puffs.MNT_NOEXEC | puffs.MNT_NODEV

local files = { hello="Hello world\n",
   hola="Hola, mundo\n",
   bonjour="Bonjour, monde\n"
}

-- that files table is unordered, but there needs to be some order
-- to do a partial readdir consistently.
-- Might as well precompute the dirents!

local function build_dents()
   local dents = {}
   local fileno = 0
   for filename in pairs(files) do
      local dent = {
	 fileno = fileno,
	 dtype = puffs.DT_REG,
	 name = filename
      }
      table.insert(dents, dent)
      fileno = fileno + 1
   end
   return dents
end
local dents = build_dents()


-- vattr is gonna be needed for both lookup and getinfo
local function build_vattr(filename)   
   local vattr = {}
   vattr.uid = 0
   vattr.gid = 0
   vattr.nlink = 1
   if filename == "/" then
      -- root directory vattr
      vattr.type = puffs.VDIR
      vattr.mode = puffs.octal("555")
      vattr.size = 16    -- why this magic number?
      vattr.bytes = 16
   else
      local filebody = files[filename]
      if filebody == nil then return nil end
      vattr.type = puffs.VREG
      vattr.mode = puffs.octal("444")
      vattr.size = #filebody
      vattr.bytes = #filebody
   end
   return vattr
end

local ops = {}

function ops:lookup(dirnode, query)
   if query.nameiop == puffs.NAMEI_LOOKUP then
      -- regular lookup
      local filename = query.name
      local filebody = files[filename]
      if filebody == nil then
	 return nil, puffs.ENOENT
      else
	 local vattr = build_vattr(filename)
	 local node = self:node_new()
	 node._key = filename
	 return {
	    node=node,
	    vtype=(puffs.VREG),
	    size=(#filebody),
	    vattr=vattr
	 }
      end
   else
      -- create/delate/rename aren't allowed in hellofs
      return nil, puffs.EACCES
   end
end
  
function ops:getattr(fnode, creds)
   local filename = fnode._key
   local vattr = build_vattr(fnode._key)
   if vattr == nil then
      -- error condition
      return nil, puffs.EPROTO
   else
      return vattr
   end
end

function ops:readdir(dirnode, offset, count, creds)
   local lbound = offset + 1

   if #dents - offset < count then
      -- overread, so truncate
      count = #dents - offset
   end

   local rbound = offset + count

   local dentlist = table.move(dents, lbound, rbound, 1, {})
   if rbound == #dents then
      -- read to end
      return dentlist
   else
      -- partial read
      return dentlist, rbound+1
   end
end

function ops:read(fnode, offset, count, flags, creds)
   if fnode == self:getroot() then
      return nil, count, true, puffs.EISDIR
   end
   
   local lbound = offset + 1
   local rbound = offset + count
   local filebody = files[fnode._key]
   local eof = (rbound >= #filebody)
   local data = filebody:substr(lbound, rbound)

   return data, (count - #data), eof
end


function main()
   local um = puffs.init(ops, fsname, pflags)
   --um:daemon()
   local root_node = um:mount(mountpoint, mflags)
   -- tag root node
   root_node._key = "/" 
   print("entering main loop!")
   um:mainloop()
   print("bye!")
end

main() 
