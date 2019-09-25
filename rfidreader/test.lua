-- enduser_setup.start(
--   function()
--     print("Connected to wifi as: " .. wifi.sta.getip())
--   end,
--   function(err, str)
--     print("enduser_setup: Err #" .. err .. ": " .. str)
--   end
-- )

nfc = require "pn532"

red = string.char(0,10,0,0,0,0)
green = string.char(10,0,0,0,0,0)
blue = string.char(0,0,10,0,0,0)
black = string.char(0,0,0,0,0,0) 
red = red..red..red
red = red..red..red..red
green = green..green..green
green = green..green..green..green
blue = blue..blue..blue
blue = blue..blue..blue..blue
black = black..black..black
black = black..black..black..black
ws2812.init()
ws2812.write(black)

nfc.init()

for i=1,10000 do
    cardid = nfc.scancard()
    if cardid == nil then
        ws2812.write(black)
    elseif cardid == 0xc07e60a8 then
        ws2812.write(green)
    elseif cardid == 0xe4c3b6c3 then
        ws2812.write(blue)
    else
        ws2812.write(red)
    end
end
ws2812.write(black)
