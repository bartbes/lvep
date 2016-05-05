function love.load()
	package.cpath = "./lib?.so"
	newStream = require "lvep"
	f = love.filesystem.newFile("sintel.mp4")
	str = newStream(f)
	vid = love.graphics.newVideo(str)
	--audio = love.audio.newSource("sintel.ogv", "stream")
	--str:setSync(audio)
	--audio:play()
	vid:play()
end

function love.draw()
	love.graphics.draw(vid)
end
