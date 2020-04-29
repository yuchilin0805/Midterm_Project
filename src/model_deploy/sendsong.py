import numpy as np
import serial
import time

waitTime = 0.1

signallength=42
song=[261, 261, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 261,
  392, 392, 349, 349, 330, 330, 294,
  392, 392, 349, 349, 330, 330, 294,
  261, 261, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 261]
name="little starff3"
formatter = lambda x: "%d" % x

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)
#print("Sending signal ...")
#print("It may take about %d seconds ..." % (int(signallength * waitTime)))
#for data in song:
#    print(data)
s.write(bytes(formatter(signallength),'UTF-8'))
s.write(bytes('\n','UTF-8'))

s.write(bytes(name,'UTF-8'))
s.write(bytes("$",'UTF-8'))


for data in song:
  s.write(bytes(formatter(data) ,'UTF-8'))
  time.sleep(waitTime)
s.close()
#print("Signal sended")