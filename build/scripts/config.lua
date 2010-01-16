local config = {}
local out = {}

local function add_option(name, type, title, desc, default)
	config[#config + 1] = {
		name    = name,
		type    = type,
		title   = title,
		desc    = desc,
		default = default,
	}
end

local function load_options(file)
	_G.OPTION = add_option
	assert(dofile(file))
	_G.OPTION = nil
end

local function get_option(opt)
	print(opt.title)
end

local function configure()
	for i, v in ipairs(config) do
		get_option(v)
	end
end

local function create_config_h()
	local f = assert(io.open("kernel/include/kos/config.h.in", "w"))
	
	for 
end


