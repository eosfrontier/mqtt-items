-- enduser_setup.start(
--   function()
--     print("Connected to wifi as: " .. wifi.sta.getip())
--   end,
--   function(err, str)
--     print("enduser_setup: Err #" .. err .. ": " .. str)
--   end
-- )

b = 2

red =   string.char(b*0,b*0,b*0, b*6,b*6,b*0, b*4,b*8,b*0, b*0,b*9,b*0, b*0,b*8,b*4, b*0,b*6,b*6)
green = string.char(b*8,b*0,b*4, b*9,b*0,b*0, b*8,b*4,b*0, b*6,b*6,b*0, b*0,b*0,b*0, b*6,b*0,b*6)
blue =  string.char(b*4,b*0,b*8, b*6,b*0,b*6, b*0,b*0,b*0, b*0,b*6,b*6, b*0,b*4,b*8, b*0,b*0,b*9)
black = string.char(b*0,b*0,b*0, b*0,b*0,b*0, b*0,b*0,b*0, b*0,b*0,b*0, b*0,b*0,b*0, b*0,b*0,b*0) 
red = red..red..red..red
green = green..green..green..green
blue = blue..blue..blue..blue
black = black..black..black..black
ws2812.init()
ws2812.write(black)
colors = {black,red,green,blue}

mytimer = tmr.create()
mycounter = 1
mytimer:register(1000, tmr.ALARM_AUTO, function()
        mycounter = mycounter+1
        if mycounter > 4 then mycounter = mycounter - 4 end
        ws2812.write(colors[mycounter])
    end)
mytimer:start()
