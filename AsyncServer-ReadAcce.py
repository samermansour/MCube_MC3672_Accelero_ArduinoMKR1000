import asyncore
import socket
from io import StringIO
import logging
import matplotlib.pyplot as plt

fig = plt.figure()
plt.ion()
ax1 = fig.add_subplot(3,1,1)
ax2 = fig.add_subplot(3,1,2)
ax3 = fig.add_subplot(3,1,3)

################################################################################
# This class is the handler that does receiving from a particular client
#One sample containing XYZ is wrapped like this "STXx_val,y_val,z_valETX"
#Note that the data was sent as a string of characters of 1 byte each (ASCII). 
#In python an ASCII character is best represented by a "bytes" variable.
#So we need to decode into a regular string. By S.M.
################################################################################
class AcceleroHandler(asyncore.dispatcher):
    
    def __init__(self, sock):
        asyncore.dispatcher.__init__(self, sock) 
        self.set_socket(sock)
        self.read_buffer = StringIO()
        self.bytesList   = []
        self.XYZSample   = "" 
        self.commas      = ""
        self.windowSize  = 100 #plotting a sliding window of ... samples
        self.X           = []
        self.Y           = []
        self.Z           = []
        print("AcceleroHandler intstantiated");

    
    def handle_read(self):
        data = self.recv(1)
        if (data):
          if (data == b'\x02'):
             #Re-initialize these for each XYZ sample
             self.XYZSample = ""
             self.commas = []
          #print (data)
          self.bytesList = self.bytesList + [data]
          curChar        = data.decode("UTF-8","strict")
          self.XYZSample = self.XYZSample + curChar
          if(curChar == ','): #Find the indices of the commas on the go
             self.commas.insert(len(self.commas), len( self.XYZSample)-1)
          if (data == b'\x03'):
             print(self.XYZSample)
             print(self.commas)
             xstr = self.XYZSample[1:self.commas[0]]
             x    = float(xstr)
             ystr = self.XYZSample[self.commas[0]+1 : self.commas[1]]
             y    = float(ystr)
             zstr = self.XYZSample[self.commas[1]+1 : len(self.XYZSample)-2]
             z = float(zstr)
          #    ystr = str(self.XYZSample[6:10])
          #    zstr = str(self.XYZSample[11:15])
          #    print(xstr +','+ ystr+','+zstr)
          #    x = float(self.XYZSample[1:5])
             print(x, " ", y, " ", z)
             #update the arrays
             self.X.insert(len(self.X),x)
             self.Y.insert(len(self.Y),y)
             self.Z.insert(len(self.Z),z)
             if(len(self.X) > self.windowSize): #drop one
                self.X.remove(self.X[0])
                self.Y.remove(self.Y[0])
                self.Z.remove(self.Z[0])
             print(self.X,'\n',self.Y,'\n', self.Z)
             ax1.clear()
             ax1.plot(self.X)
             ax2.clear()
             ax2.plot(self.Y)
             ax3.clear()
             ax3.plot(self.Z)
             plt.pause(0.05)                  
################################################################################
#This class works is a server waiting to get connections from the 
#microcontroller. It hands the socket to the AcceleroHandler which will handle
#receiving the data. 
################################################################################
class AcceleroServer(asyncore.dispatcher):

    def __init__(self, host, port):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(5)

    def handle_accept(self):
        pair = self.accept()
        if pair is None:
            return
        else:    
            sock, addr = pair
            print ('Incoming connection from %s'  % repr(addr))
            handler = AcceleroHandler(sock)
           
    def handle_close(self):
        print('handle_close()')
        self.close()        
################################################################################
server = AcceleroServer ('192.168.8.105', 5555)#('172.20.10.7', 5555)#('10.53.0.160', 5555)
asyncore.loop()
