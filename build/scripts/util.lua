local function parse_params(opts, ...)
	for i=1, select('#', ...) do
		local p = select(i, ...)
		opts[p] = true
	end
end

local PATH_SEP = "\\"

local SEARCH_PREFIX = "kernel" .. PATH_SEP
local OUT_PREFIX    = "build" .. PATH_SEP .. "objs" .. PATH_SEP

local ignore_list = {
	["include"] = true,
}
local function Ignored(dir)
	for mask in pairs(ignore_list) do
		if dir:match(mask) then
			return true
		end
	end
	return false
end

local exec = os.execute

local function CreateDirectory(path)
	return exec("mkdir " .. path)
end

local function RemoveDirectory(path)
	return exec("rm -r " .. path)
end

local function IterateDirectories(path, func, arg)
	local list = assert(io.popen("DIR /A:D /B " .. path))
	for line in list:lines() do
		if not Ignored(line) then
			func(line, arg)
		end
	end
end

local CreateBuildDir
function CreateBuildDir(cur, base)
	CreateDirectory(OUT_PREFIX .. base..cur)
	IterateDirectories(SEARCH_PREFIX .. base..cur, CreateBuildDir, base .. cur .. PATH_SEP)
end

local function CreateObjectDirs()
	IterateDirectories(SEARCH_PREFIX, CreateBuildDir, "")
end

local opts = {}
parse_params(opts, ...)

if opts.prepare then
	CreateDirectory(OUT_PREFIX)
	CreateObjectDirs()
end

if opts.cleanup then
	RemoveDirectory(OUT_PREFIX)
end
