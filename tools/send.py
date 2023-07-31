import serial
import sys

ser = serial.Serial('/dev/ttyUSB0', 115200)
print(ser.name) 


if(len(sys.argv) > 2):
    f = open(sys.argv[1], "rb")
    f2 = open(sys.argv[2], "rb")
    data_i = f.read()
    data_d = f2.read()
else:
    print ("Expcected usage: send.py <text bin file> <data bin file>")
    exit(1)

def send(data, addr):
    dbytes = len(data)
    ser.write(ord('m').to_bytes(1, byteorder='little'))
    ser.write(dbytes.to_bytes(2, byteorder='little'))
    print(dbytes.to_bytes(2, byteorder='little'))
    ser.write(addr.to_bytes(4, byteorder='little'))
    print(addr.to_bytes(4, byteorder='little'))
    
    ser.write(data)

    checksum = 0
    for i in range(0, dbytes, 2):
        checksum += int.from_bytes(data[i:i+2], byteorder='little')
        checksum %= (1<<16)
    print("cs", hex(checksum)) 

# send program 
send(data_i, (0x800000 << 1))
# send data
send(data_d, (0x100000 << 1) + (0x1000))

# execute
ser.write(ord('e').to_bytes(1, byteorder='little'))
