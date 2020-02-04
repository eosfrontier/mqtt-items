let leds = require("animator").init(3,"180000,001800,000020")
let msgr = require("messager").init(leds,"eos/portal/buttons_out")
let btns = require("buttons").init(msgr,{14:"b1",12:"b2",13:"b3"})
