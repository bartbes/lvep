// STL
#include <iostream>
#include <fstream>

// Lua
#include <lua.hpp>

// LOVE
#include <common/runtime.h>

// LVEP
#include "FFMpegStream.h"
#include "LVEPVideoStream.h"

int w_newVideoStream(lua_State *L)
{
	auto file = love::luax_checktype<love::filesystem::File>(L, 1, love::FILESYSTEM_FILE_ID);
	love::luax_catchexcept(L, [&]() {
		love::video::VideoStream *stream = new LVEPVideoStream(file);
		luax_pushtype(L, love::VIDEO_VIDEO_STREAM_ID, stream);
		stream->release();
	});
	return 1;
}

extern "C" int luaopen_lvep(lua_State *L)
{
	av_register_all();

	lua_pushcfunction(L, w_newVideoStream);
	return 1;
}
