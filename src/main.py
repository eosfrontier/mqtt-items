from utime import sleep_ms, ticks_ms, ticks_diff
import wifiportal,ujson
import leds, buttons, mqtt

f = open('settings.json', 'r')
configs = ujson.loads(f.read())
print(configs)
f.close()
f = open('password.json', 'r')
pwd = ujson.loads(f.read())
f.close()
Leds = leds.Leds(configs['leds'])
Btns = buttons.Buttons(configs['buttons'])
MQTT = mqtt.MQTT(configs['mqtt'], pwd, Leds, Btns)

ms_per_frame = int(1000 / configs['fps'])

while True:
    now = ticks_ms()
    try:
        MQTT.check()
        Leds.animate()
    except KeyboardInterrupt:
        print('Break')
        break
    except Exception as e:
        print(e)
        pass
    rest = ticks_diff(ticks_ms(),now)
    MQTT.loadavg = (MQTT.loadavg * 0.99) + ((rest/ms_per_frame) * 0.01)
    if rest < ms_per_frame:
        sleep_ms(ms_per_frame-rest)
