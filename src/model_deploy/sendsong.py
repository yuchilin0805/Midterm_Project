import numpy as np
import serial
import time

waitTime = 0.1

songlength1=42
song1=[261, 261, 392, 392, 440, 440, 392,
      349, 349, 330, 330, 294, 294, 261,
      392, 392, 349, 349, 330, 330, 294,
      392, 392, 349, 349, 330, 330, 294,
      261, 261, 392, 392, 440, 440, 392,
      349, 349, 330, 330, 294, 294, 261]
note1=[1, 1, 1, 1, 1, 1, 2,
      1, 1, 1, 1, 1, 1, 2,
      1, 1, 1, 1, 1, 1, 2,
      1, 1, 1, 1, 1, 1, 2,
      1, 1, 1, 1, 1, 1, 2,
      1, 1, 1, 1, 1, 1, 2]
name1="little starff3"


songlength2=49
song2=[392,330,330,349
,294,294,261,294,330,349,392,392,392,392,330,330,349
,294,294,261,330,392,392,330,294,294,294,294,294,330,349,330,
330,330,330,330,349,392,392,330,330,349,294,294,261,330,392,392,261]

note2=[1,1,2,1,1,2,1,1,1,1,1,1,2,
       1,1,2,1,1,2,1,1,1,1,4,
       1,1,1,1,1,1,2,1,1,1,1,1,1,2,
       1,1,2,1,1,2,1,1,1,1,4]
name2="little beesss"

formatter = lambda x: "%d" % x

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)
#print("Sending signal ...")
#print("It may take about %d seconds ..." % (int(signallength * waitTime)))
#for data in note2:
  # print(data)
#print(songlength2)
s.flushOutput()

s.write(bytes("#",'UTF-8'))
s.write(bytes(formatter(songlength2),'UTF-8'))
time.sleep(waitTime)
#s.write(bytes('\0','UTF-8'))
time.sleep(waitTime)
s.write(bytes(name2,'UTF-8'))
time.sleep(waitTime)
s.write(bytes("$",'UTF-8'))


for data in song2:
  s.write(bytes(formatter(data) ,'UTF-8'))
  time.sleep(waitTime)
for data in note2:
  s.write(bytes(formatter(data) ,'UTF-8'))
  time.sleep(waitTime)
#s.write(bytes("ffffff",'UTF-8'))
s.close()
#print("Signal sended")