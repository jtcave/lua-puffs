-- quick protoype of a hello world FS

-- kludge for quick testing
package.cpath = "./?.so"
puffs = require 'puffs'

-- node tags
path_tags = {}
setmetatable(path_tags, {__mode = "k"})

fsname = "hellofs"
mountpoint = "/mnt"
pflags = 0 --puffs.PUFFS_KFLAG_NOCACHE
mflags = puffs.MNT_RDONLY | puffs.MNT_NOEXEC | puffs.MNT_NODEV

local files = {
   hello="Hello World\n",
   hola="Hola Mundo\n",
   bonjour="Bonjour le monde\n",
   ola="Ola Mundo\n",
   hallo="Hallo Welt\n"
}

-- that files table is unordered, but there needs to be some order
-- to do a partial readdir consistently.
-- Might as well precompute the dirents!

local function build_dents()
   local dents = {
      {
	 fileno = 0,
	 dtype = puffs.DT_DIR,
	 name = "."
      }
   }
   local fileno = 1
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
   --print("ops:lookup()")
   if query.nameiop == puffs.NAMEI_LOOKUP then
      -- regular lookup
      local filename = query.name
      local filebody = files[filename]
      if filebody == nil then
	 return nil, puffs.ENOENT
      else
	 local vattr = build_vattr(filename)
	 local node = self:node_new()
	 path_tags[node] = filename
	 local retval = {
	    node = node,
	    vtype = (puffs.VREG),
	    size = (#filebody),
	    vattr = vattr
	 }
	 return retval
      end
   else
      -- create/delate/rename aren't allowed in hellofs
      return nil, puffs.EACCES
   end
end
  
function ops:getattr(fnode, creds)
   --print("ops:getattr()", fnode, path_tags[fnode])
   local filename = path_tags[fnode]
   local vattr = build_vattr(filename)
   if vattr == nil then
      -- error condition
      return nil, puffs.EPROTO
   else
      return vattr
   end
end

function ops:readdir(dirnode, offset, count, creds)
   --print("ops:readdir()")
   --print("dirnode", dirnode, path_tags[dirnode])
   --print("offset", offset)
   --print("count", count)
   --print("creds", creds)
   local lbound = offset + 1

   if #dents - offset < count then
      -- overread, so truncate
      count = #dents - offset
   end

   local rbound = offset + count

   local dentlist = table.move(dents, lbound, rbound, 1, {})
   --for i,v in ipairs(dentlist) do
   --   print(i, v, v.name)
   --end
   if rbound == #dents then
      -- read to end
      return dentlist
   else
      -- partial read
      return dentlist, rbound+1
   end
end

function ops:read(fnode, offset, count, flags, creds)
   --print("ops:read()", path_tags[fnode])
   if fnode == root_node then
      return nil, count, true, puffs.EISDIR
   end
   
   local lbound = offset + 1
   local rbound = offset + count
   
   local filebody = files[path_tags[fnode]]
   local eof = (rbound >= #filebody)
   local data = filebody:sub(lbound, rbound)

   return data, eof
end


function main()
   local um = puffs.init(ops, fsname, pflags, fsname)
   --um:daemon()
   root_node = um:mount(mountpoint, mflags)
   -- tag root node
   path_tags[root_node] = "/"

   print("entering main loop!")
   um:mainloop()
   print("bye!")
end

main() 
