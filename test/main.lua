function love.load()
	package.cpath = "./lib?.so"
	newVideoStream, newDecoder = unpack(require "lvep")
	do
		local f = love.filesystem.newFile("sintel.mp4")
		vs = newVideoStream(f)
	end
	do
		local f = love.filesystem.newFile("sintel.mp4")
		d = newDecoder(f)
	end
	vid = love.graphics.newVideo(vs)
	aud = love.audio.newSource(d)
	vs:setSync(aud)
	aud:play()
end

function love.keypressed(key)
	if key == "escape" then
		return love.event.quit()
	elseif key == "space" then
		aud:seek(5)
		if not aud:isPlaying() then
			aud:play()
		end
	end
end

function love.draw()
	love.graphics.draw(vid)
end
