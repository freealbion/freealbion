
-- vars
local cygwin = false

-- this function returns the first result of "find basepath -name filename", this is needed on some platforms to determine the include path of a library
function find_include(filename, base_path)
	if(os.is("windows") and not cygwin) then
		return ""
	end
	
	local proc = io.popen("find "..base_path.." -name \""..filename.."\"", "r")
	local path_names = proc:read("*a")
	proc:close()
	
	if(string.len(path_names) == 0) then
		return ""
	end
	
	local newline = string.find(path_names, "\n")
	if newline == nil then
		return ""
	end
	
	return string.sub(path_names, 0, newline-1)
end


-- actual premake info
solution "albion"
	configurations { "Release", "Debug" }

project "albion"
	targetname "albion"
	kind "ConsoleApp"
	language "C++"
	files { "src/**.h", "src/**.cpp" }
	platforms { "x32", "x64" }
	defines { "A2E_NET_PROTOCOL=TCP_protocol" }
	targetdir "bin"
	
	-- scan args
	local argc = 1
	while(_ARGS[argc] ~= nil) do
		if(_ARGS[argc] == "--env") then
			argc=argc+1
			-- check if we are building with cygwin/mingw
			if(_ARGS[argc] ~= nil and _ARGS[argc] == "cygwin") then
				cygwin = true
			end
		end
		argc=argc+1
	end

	if(not os.is("windows") or cygwin) then
		if(not cygwin) then
			includedirs { "/usr/include" }
		else
			includedirs { "/usr/include/w32api", "/usr/include/w32api/GL" }
		end
		includedirs { "/usr/include/freetype2", "/usr/include/libxml2", "/usr/local/include", "/usr/local/include/a2e" }
		buildoptions { "-Wall -x c++ -fmessage-length=0 -pipe -Wno-trigraphs -Wreturn-type -Wunused-variable -funroll-loops" }
		buildoptions { "-msse3" }
	end
	
	if(cygwin) then
		-- only works with gnu++0x for now ...
		buildoptions { "-std=gnu++0x" }
		defines { "CYGWIN" }
	end
	
	if(os.is("linux") or os.is("bsd") or cygwin) then
		-- find lua lib (try different lib names)
		local lua_lib_names = { "lua", "lua-5.1", "lua5.1" }
		local lua_lib = { name = nil, dir = nil }
		for i = 1, table.maxn(lua_lib_names) do
			lua_lib.name = lua_lib_names[i]
			lua_lib.dir = os.findlib(lua_lib.name)
			if(lua_lib.dir ~= nil) then
				break
			end
		end
		
		libdirs { os.findlib("lua"), os.findlib("ftgl"), os.findlib("xml2"), os.findlib("a2e") }
		links { lua_lib.name, "xml2" }
		if(not cygwin) then
			links { "GL", "SDL_net", "SDL_image", "ftgl" }
			libdirs { os.findlib("SDL"), os.findlib("SDL_net"), os.findlib("SDL_image"), lua_lib.dir }
			buildoptions { "`sdl-config --cflags`" }
			linkoptions { "`sdl-config --libs`" }
		else
			links { "opengl32", "SDL_net.dll", "SDL_image.dll", "ftgl.dll" }
			libdirs { "/usr/lib/w32api" }
			buildoptions { "`sdl-config --cflags | sed -E 's/-Dmain=SDL_main//g'`" }
			linkoptions { "`sdl-config --libs | sed -E 's/(-lmingw32|-mwindows)//g'`" }
		end
		defines { "_GLIBCXX__PTHREADS" }
		
		-- find all necessary headers (in case they aren't in /usr/include)
		local include_files = { "SDL.h", "SDL_net.h", "lua.h" }
		for i = 1, table.maxn(include_files) do
			if((not os.isfile("/usr/include/"..include_files[i])) and
			   (not os.isfile("/usr/local/include/"..include_files[i]))) then
				-- search in /usr/include and /usr/local/include
				local include_path = find_include(include_files[i], "/usr/include/")
				if(include_path == "") then
					include_path = find_include(include_files[i], "/usr/local/include/")
				end
				
				if(include_path ~= "") then
					includedirs { path.getdirectory(include_path) }
				end
			end
		end
	end
	
	-- use provided xcode project for now
	--[[if(os.is("macosx")) then
		buildoptions { "-Iinclude -I/usr/local/include -isysroot /Developer/SDKs/MacOSX10.6.sdk -msse4.1 -mmacosx-version-min=10.6 -gdwarf-2 -mdynamic-no-pic" }
		linkoptions { "-isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -framework SDL_net -framework SDL" }
	end]]--
	
	if(os.is("windows") and not cygwin) then
		links { "opengl32", "glu32", "odbc32", "odbccp32", "SDL", "SDLmain", "SDL_net", "SDL_image", "ftgl", "lua51", "libxml2", "vcomp", "OpenCL" }
		defines { "__WINDOWS__", "_CONSOLE", "A2E_IMPORTS", "_CRT_SECURE_NO_DEPRECATE" }
		flags { "NoMinimalRebuild", "NoEditAndContinue" }
		buildoptions { "/Zi /MP8" }
	end
	
	-- the same for all
	includedirs { "src/", "src/gfx/", "src/gfx/hqx/", "src/map/", "src/map/2d/", "src/map/3d/", "src/ui/", "src/events/" }
	
	-- configs
	configuration { "x32" }
		defines { "PLATFORM_X86" }
	
	configuration { "x64" }
		defines { "PLATFORM_X64" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }
		if(os.is("macosx")) then
			linkoptions { "-la2ed" }
		else
			links { "a2ed" }
		end

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }
		if(not os.is("windows") or cygwin) then
			buildoptions { "-ffast-math -ftracer -O3 -frename-registers -fweb -finline-functions -O3" }
		end
		if(os.is("macosx")) then
			linkoptions { "-la2e" }
		else
			links { "a2e" }
		end
