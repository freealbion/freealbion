
-- vars
local win_unixenv = false
local cygwin = false
local mingw = false
local clang_libcxx = false

-- this function returns the first result of "find basepath -name filename", this is needed on some platforms to determine the include path of a library
function find_include(filename, base_path)
	if(os.is("windows") and not win_unixenv) then
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
	platforms { "x64", "x32" }
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
				win_unixenv = true
			end
			if(_ARGS[argc] ~= nil and _ARGS[argc] == "mingw") then
				mingw = true
				win_unixenv = true
			end
		end
		if(_ARGS[argc] == "--clang") then
				clang_libcxx = true
		end
		argc=argc+1
	end

	if(not os.is("windows") or win_unixenv) then
		if(not cygwin) then
			includedirs { "/usr/include" }
		else
			includedirs { "/usr/include/w32api", "/usr/include/w32api/GL" }
		end
		includedirs { "/usr/include/freetype2", "/usr/include/libxml2", "/usr/local/include", "/usr/include/a2e" }
		buildoptions { "-Wall -x c++ -std=c++0x -Wno-trigraphs -Wreturn-type -Wunused-variable -funroll-loops" }
		if(clang_libcxx) then
			buildoptions { "-stdlib=libc++" }
			buildoptions { "-Wno-delete-non-virtual-dtor -Wno-overloaded-virtual" }
			linkoptions { "-stdlib=libc++" }
		end
	end
	
	if(win_unixenv) then
		-- only works with gnu++0x for now ...
		buildoptions { "-std=gnu++0x" }
		defines { "WIN_UNIXENV" }
		if(cygwin) then
			defines { "CYGWIN" }
		end
		if(mingw) then
			defines { "__WINDOWS__", "MINGW" }
			includedirs { "/mingw/include/GL" }
			libdirs { "/usr/lib", "/usr/local/lib" }
			buildoptions { "-Wno-unknown-pragmas" }
		end
	end
	
	if(os.is("linux") or os.is("bsd") or win_unixenv) then
		libdirs { os.findlib("ftgl"), os.findlib("xml2"), os.findlib("a2e") }
		if(not win_unixenv) then
			links { "GL", "SDL_net", "SDL_image", "ftgl", "xml2" }
			libdirs { os.findlib("SDL"), os.findlib("SDL_net"), os.findlib("SDL_image") }
			buildoptions { "`sdl-config --cflags`" }
			linkoptions { "`sdl-config --libs`" }
		elseif(cygwin) then
			-- link against windows opengl libs on cygwin
			links { "opengl32", "SDL_net.dll", "SDL_image.dll", "ftgl.dll", "xml2" }
			libdirs { "/lib/w32api" }
			buildoptions { "`sdl-config --cflags | sed -E 's/-Dmain=SDL_main//g'`" }
			linkoptions { "`sdl-config --libs | sed -E 's/(-lmingw32|-mwindows)//g'`" }
		elseif(mingw) then
			-- link against windows opengl libs on mingw
			links { "opengl32", "glu32", "SDL_net", "SDL_image", "ftgl", "libxml2" }
			buildoptions { "`sdl-config --cflags | sed -E 's/-Dmain=SDL_main//g'`" }
			linkoptions { "`sdl-config --libs`" }
		end
		
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
	
	if(os.is("windows") and not win_unixenv) then
		links { "opengl32", "glu32", "odbc32", "odbccp32", "SDL", "SDLmain", "SDL_net", "SDL_image", "ftgl", "libxml2", "vcomp", "OpenCL" }
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
		links { "a2ed" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }
		if(not os.is("windows") or win_unixenv) then
			buildoptions { "-ffast-math -O3" }
		end
		links { "a2e" }
