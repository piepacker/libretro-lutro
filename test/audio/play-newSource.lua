local playTime = 0.75
local timer = 0

local soundfiles = {
	"audio/test_s16_mono.wav",
	"audio/test_float32_mono.wav",
	"audio/test_ogg_mono.ogg"
}

local sources = {}
local cursource = 0

return {
	load = function()
		for i,file in ipairs(sourcefiles)
			print ("  loading " .. file)
			sources[i] = lutro.audio.newSource(file)
		end
	end,

	update = function(dt)
		timer = timer + dt
		if timer > playTime then
			print ("  playing " .. soundfiles[cursource])
			lutro.audio.play(sources[cursource])
			cursource = cursource + 1
			if (cursource >= #sources) then
				cursource = 0
			end
			timer = 0
		end
	end
}
