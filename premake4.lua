
-- this function returns the first result of "find basepath -name filename", this is needed on some platforms to determine the include path of a library
function find_include(filename, base_path)
	if(os.is("windows")) then
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
	configurations { "Debug", "Release" }

project "albion"
	targetname "albion"
	kind "ConsoleApp"
	language "C++"
	files { "src/**.h", "src/**.cpp" }
	platforms { "x32", "x64" }
	defines { "A2E_NET_PROTOCOL=TCP_protocol", "A2E_USE_OPENMP" }
	targetdir "bin"

	-- TODO: write unix premake file
	--[[if(not os.is("windows")) then
		includedirs { "/usr/include", "/usr/local/include", "./", "src/", "src/control/", "src/db/", "src/net/", "src/plugins/", "src/plugins/implementations", "src/threading/" }
		buildoptions { "-Wall -x c++ -fmessage-length=0 -pipe -Wno-trigraphs -Wreturn-type -Wunused-variable -funroll-loops -ftree-vectorize" }
		buildoptions { "-msse3 -fvisibility=hidden -fvisibility-inlines-hidden" }
		prebuildcommands { "./build_version.sh" }
	end
	
	if(os.is("linux") or os.is("bsd")) then
		libdirs { os.findlib("SDL"), os.findlib("SDL_net"), os.findlib("libpq") }
		links { "SDL", "SDLmain", "SDL_net", "libpq" }
		buildoptions { "`sdl-config --cflags`" }
		linkoptions { "`sdl-config --libs`" }
		defines { "_GLIBCXX__PTHREADS" }
		
		-- find all necessary headers (in case they aren't in /usr/include)
		local include_files = { "SDL.h", "SDL_net.h", "libpq-fe.h" }
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
	
	if(os.is("macosx")) then
		buildoptions { "-Iinclude -I/usr/local/include -isysroot /Developer/SDKs/MacOSX10.6.sdk -msse4.1 -mmacosx-version-min=10.6 -gdwarf-2 -mdynamic-no-pic" }
		linkoptions { "-isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -framework SDL_net -framework SDL" }
		links { "libpq" }
	end]]--
	
	if(os.is("windows")) then
		links { "opengl32", "glu32", "odbc32", "odbccp32", "SDL", "SDLmain", "SDL_net", "SDL_image", "ftgl", "lua51", "libxml2", "vcomp", "OpenCL" }
		includedirs { "src/", "src/gfx/", "src/gfx/hqx/", "src/map/", "src/map/2d/", "src/map/3d/", "src/ui/" }
		defines { "__WINDOWS__", "_CONSOLE", "A2E_IMPORTS", "_CRT_SECURE_NO_DEPRECATE" }
	end
	
	
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
		if(not os.is("windows")) then
			buildoptions { "-ffast-math -ftracer -O3 -frename-registers -fweb -finline-functions -O3" }
		end
		if(os.is("macosx")) then
			linkoptions { "-la2e" }
		else
			links { "a2e" }
		end
