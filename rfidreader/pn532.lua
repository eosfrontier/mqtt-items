-- PN532 module, om mifare kaart-IDs te lezen
local pn532 = {}

local function printhex(txt, ...)
    uart.write(0, txt)
    for _,c in ipairs(arg) do
        uart.write(0, string.format(" %02x", c))
    end
    print()
end

local function sendframe(cmd, ...)
    -- Build PN532 frame
    frame = {0x00, 0xff, #arg+2, 254-#arg, 0xd4, cmd}
    chk = -0xd4 - cmd
    for _,c in ipairs(arg) do
        table.insert(frame, c)
        chk = chk - c
    end
    -- Calc checksum, end with 0
    chk = chk % 256
    table.insert(frame, chk)
    table.insert(frame, 0)
    -- Send frame
    i2c.start(0)
    -- print("Send address")
    if i2c.address(0, 0x24, i2c.TRANSMITTER) == false then
        i2c.stop(0)
        print(string.format("Failed to send address for %02x",cmd))
        return false
    end
    -- printhex("Send frame: ", unpack(frame))
    cnt = i2c.write(0, unpack(frame))
    i2c.stop(0)
    if cnt < #frame then
        print(string.format("Failed to write %d bytes for %02x",#frame, cmd))
        return false
    end
    -- Receive ACK frame
    i2c.start(0)
    if i2c.address(0, 0x24, i2c.RECEIVER) == false then
        i2c.stop(0)
        print(string.format("Failed to receive address for %02x",cmd))
        return false
    end
    -- print "Read ACK"
    ack = i2c.read(0, 7)
    i2c.stop(0)
    if ack == string.char(0x01, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00) then
        return true
    end
    print(string.format("Did not get ACK for %02x", cmd))
    printhex("ACK was: ", string.byte(ack, 1, 7))
    printhex("Frame was: ", unpack(frame))
    return false
end

local function readframe(cmd,count,timeout)
    while timeout > 0 do
        i2c.start(0)
        if i2c.address(0, 0x24, i2c.RECEIVER) == false then
            i2c.stop(0)
            return nil
        end
        result = i2c.read(0, count+8)
        i2c.stop(0)
        if #result >= 1 and result:byte(1) == 1 then
            if result:byte(8)-1 ~= cmd then
                print(string.format("Unexpected command %02x<>%02x",cmd,result:byte(7)-1))
                printhex("Result:", result:byte(1, count+8))
                return nil
            end
            return {result:byte(9,#result)}
        end
        timeout = timeout - 1
        tmr.delay(1)
    end
    return nil
end

function pn532.getfirmware()
    if not sendframe(0x02) then return false end
    res = readframe(0x02, 4, 10)
    if res == nil then return false end
    print(string.format("Firmware: IC:%02x Ver:%d.%d Supported:%02x", res[1], res[2], res[3], res[4]))
    return true
end

function pn532.configsam(...)
    if not sendframe(0x14, unpack(arg)) then return false end
    res = readframe(0x14, 0, 10)
    if res == nil then return false end
    return true
end

function pn532.configretry(maxretry)
    if not sendframe(0x32, 0x05, 0x02, 0x02, maxretry) then return false end
    res = readframe(0x32, 0, 10)
    if res == nil then return false end
    return true
end

function pn532.releasecard(idx)
    if not sendframe(0x52, idx) then return false end
    res = readframe(0x52, 1, 10)
    if res == nil then return false end
    return true
end

function pn532.scancard(timeout)
    if timeout == nil then timeout = 100 end
    -- print(string.format("Scanning card"))
    if not sendframe(0x4a, 0x01, 0x00) then return nil end
    -- print(string.format("Reading (timeout %d)", timeout))
    scanres = readframe(0x4a, 10, timeout)
    if scanres == nil then return nil end
    if scanres[1] ~= 1 then return nil end
    -- pn532.releasecard(scanres[2])
    return scanres[7]*0x1000000+scanres[8]*0x10000+scanres[9]*0x100+scanres[10]
end

function pn532.init()
    i2c.setup(0, 3, 2, i2c.SLOW)
    if not pn532.configsam(1, 20, 1) then return false end
    if not pn532.configretry(10) then return false end
    return true
end

return pn532
