import serial
import sys
from time import sleep
ser = serial.Serial('/dev/ttyUSB0', 115200)
print(ser.name) 


if(len(sys.argv) > 1):
    f = open(sys.argv[1])
    data = f.read()
else:
    data = input("send: ") #ach.. this doesn't work when reading > 4095 chars! (see linux termios icannon)

it = 0; pit=0
for c in data:
    ser.write(bytes([ord(c)]))
    it = it+1
    if(int(it/(len(data)/100)) != int(pit/(len(data)/100))) :
        print(int(it/(len(data)/100)), end = " ", flush = True)
    pit = it
print("done")

ser.close()
