local cmd = ...

-- Get the host system
local system
local uname = io.popen("uname");
local sysname = uname:read("*l");
if sysname == "Linux" then
	system = "Linux"
else
	system = "Windows"
end

------
local DEBUG = false

local SILENT 
if system == "Linux" then
	SILENT = "2>/dev/null"
else
	SILENT = "2>nul"
end

local OUT_PREFIX = "build/objs/"

local SRC_DIRS = {
	["kernel/"] = true,
	["driver/"] = true,
}

local function _path(p)
	if system == "Windows" then
		return p:gsub("/", "\\")
	end
	return p
end

local function exec(cmd)
	if DEBUG then print(cmd) end
	return os.execute(cmd)
end

local function list_path(path)
	return ("ls -1 -d %s*/ %s"):format(path, SILENT)
end

local function recurse(path, func)
	local list = assert(io.popen(list_path(path)))
	for line in list:lines() do
		if not line:find("include") then
			func(line)
			recurse(line, func)
		end
	end
end

local function mkdir(path)
	return exec(("mkdir %s %s"):format(_path(path), SILENT))
end

local function make_build_dir(path)
	mkdir(OUT_PREFIX..path)
end

local function rmdir(path)
	return exec(("rm -r %s %s"):format(_path(path), SILENT))
end

------
if cmd == "prep" then
	mkdir(OUT_PREFIX)
	for dir in pairs(SRC_DIRS) do
		make_build_dir(dir)
		recurse(dir, make_build_dir)
	end
elseif cmd == "clean" then
	rmdir(OUT_PREFIX)
else
	print("Unknown command : " .. cmd or '<nil>');
end
