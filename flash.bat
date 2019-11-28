set PORT=%1

curl -L micropython.org/resources/firmware/esp8266-20190529-v1.11.bin -o esp8266-20190529-v1.11.bin

esptool.py --port %PORT% erase_flash
esptool.py --port %PORT% --baud 460800 write_flash --flash_size=detect -fm dio 0 esp8266-20190529-v1.11.bin

set DIR=%2
pushd %DIR% || exit /b 0
for %%p in (*.py,*.html,settings.json) ;do ampy -p %PORT% put %%p
popd
