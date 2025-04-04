from machine import SPI, Pin, I2C
import sdcard, uos
from time import ticks_ms
from lcd_api import LcdApi
from pico_i2c_lcd import I2cLcd
from time import sleep


spi = SPI(1,sck=Pin(14), mosi=Pin(15), miso=Pin(12))
cs = Pin(13)
doorSignal = Pin(11, Pin.IN, Pin.PULL_UP)
closeSignal = Pin(10, Pin.IN, Pin.PULL_UP)
cruseSignal = Pin(9, Pin.IN, Pin.PULL_UP)
secondSart = 0
i2c = I2C(0, sda=Pin(0), scl=Pin(1), freq=400000)
lcd = I2cLcd(i2c, 0x27, 2, 16)  

mode = 1
routeName = "test"
routeIndex = -1
routeEvent = {}

"""
Setup
"""
lcd.clear()
lcd.putstr("HEVBSZ by")
lcd.move_to(9,1)
lcd.putstr("BenceIT")
sleep(2)
try:
    sd = sdcard.SDCard(spi, cs)
    uos.mount(sd, '/data')
except OSError:
    lcd.clear()
    lcd.move_to(0,0)
    lcd.putstr("Sdcard not found")
    while True:
        pass
#print(uos.listdir('/data/sounds'))


"""
Loop
"""
def numconverter(number):
    return "{:03d}".format(int(number)) 

while True:
    if mode == 1:
        temp = 0
        with open(f"/data/routes/{routeName}.txt") as routeFile:
            for index in routeFile:
                if temp == routeIndex+1:
                    index = str(index)
                    index = index.split(",")
                    index [-1] = index [-1] [1]
                    routeEvent = {
                        'displayText':index[0].strip(),
                        'trigger':index[1].strip(),
                        'timer':int(index[2].strip()),
                        'soundName': index[3].strip(),
                        'stationCode': index[4].strip(),
                        'station': index[5].strip() == "1",
                        'busy': index[6].strip() == "1",
                        'endflag': index[7].strip() == "1",
                    }
                    break
                else: temp+=1
            routeFile.close()
        
        lcd.clear()
        lcd.move_to(0,0)
        lcd.putstr(routeEvent['displayText'])
        lcd.move_to(13,1)
        lcd.putstr(numconverter(routeEvent["timer"]))

        routeIndex += 1
        mode = 2
    if mode == 2:
        if routeEvent['trigger'] == 'A' and doorSignal.value() == 1:
            mode = 3
            secondSart = ticks_ms()
        elif routeEvent['trigger'] == 'Z' and closeSignal.value() == 1:
            mode = 3
            secondSart = ticks_ms()
        elif routeEvent['trigger'] == 'M' and cruseSignal.value() == 1:
            mode = 3
            secondSart = ticks_ms()
    if mode == 3:
        if ticks_ms() - secondSart >= 1000:
            if routeEvent['timer'] != 0:
                secondSart = ticks_ms()
                routeEvent['timer'] -= 1
                lcd.move_to(13,1)
                lcd.putstr(numconverter(routeEvent["timer"]))
            else: mode = 4
    if mode == 4:
        mode = 1

    lcd.move_to(8,1)
    if cruseSignal.value() == 1: lcd.putstr('M')
    else: lcd.putstr('-')
    if doorSignal.value() == 1: lcd.putstr('A')
    else: lcd.putstr('-')
    if closeSignal.value() == 1: lcd.putstr('Z')
    else: lcd.putstr('-')