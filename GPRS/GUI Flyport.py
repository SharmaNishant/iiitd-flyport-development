from tkinter import *

import serial
import time

def Initialize():
    MainFrame2.grid_remove()

    MainFrame1.grid(row=0)
    LabelEnterDetails1.grid(column=0, columnspan=2, row=0)
    LabelCOMPort.grid(column=0, row=1)
    gLabelCOMPort.grid(column=1, row=1)
    ConnectButton.grid(column=0, row=2, columnspan=2)


def ReInitialize():
    MainFrame1.grid_remove()

    MainFrame2.grid(row=0)
    LabelEnterDetails2.grid(column=0, columnspan=2, row=0)
    SPeriod.grid(column=0, row=1)
    gSPeriod.grid(column=1, row=1)
    PPeriod.grid(column=0, row=2)
    gPPeriod.grid(column=1, row=2)
    APIKey.grid(column=0, row=3)
    gAPIKey.grid(column=1, row=3)
    DeviceName.grid(column=0, row=4)
    gDeviceName.grid(column=1, row=4)
    Location.grid(column=0, row=5)
    gLocation.grid(column=1, row=5)
    ServerIP.grid(column=0, row=6)
    gServerIP.grid(column=1, row=6)
    ServerPort.grid(column=0, row=7)
    gServerPort.grid(column=1, row=7)
    ServerURL.grid(column=0, row=8)
    gServerURL.grid(column=1, row=8)
    APN.grid(column=0, row=9)
    gAPN.grid(column=1, row=9)
    Login.grid(column=0, row=10)
    gLogin.grid(column=1, row=10)
    Password.grid(column=0, row=11)
    gPassword.grid(column=1, row=11)
    IPAddress.grid(column=0, row=12)
    gIPAddress.grid(column=1, row=12)
    DNS1.grid(column=0, row=13)
    gDNS1.grid(column=1, row=13)
    DNS2.grid(column=0, row=14)
    gDNS2.grid(column=1, row=14)
    UpdateButton.grid(column=0, row=15, columnspan=2)

def GetStringFromBytes(par):
    ret=''
    for i in range(0, len(par)) :
        ret += chr(par[i])
    return ret

def PutValues(a):
    b = a.split('\r\n')

    gSPeriod.insert(0, b[1])
    gPPeriod.insert(0, b[2])
    gAPIKey.insert(0, b[3])
    gDeviceName.insert(0, b[4])
    gLocation.insert(0, b[5])
    gServerIP.insert(0, b[6])
    gServerPort.insert(0, b[7])
    gServerURL.insert(0, b[8])
    gAPN.insert(0, b[9])
    gLogin.insert(0, b[10])
    gPassword.insert(0, b[11])
    gIPAddress.insert(0, b[12])
    gDNS1.insert(0, b[13])
    gDNS2.insert(0, b[14])

def Connect():
    try:
        global COMPort
        COMPort = gLabelCOMPort.get()
        ser = serial.Serial(COMPort, baudrate=19200)
        print(ser)

        StatusLabel.set("Serial Port:'" + COMPort + "' Open")
        print(StatusLabel.get())

        ReInitialize()

        StatusLabel.set("Serial Port:'" + COMPort + "' Open")
        print(StatusLabel.get())

        ser.write(bytes('@fetch\r\n', 'utf-8'))

        string = ''
        while string != '@values\r\n':
            while ser.inWaiting == 0:
               continue

            string = GetStringFromBytes(ser.readline())
            print(string)

            final=string

        while string != '@end-values\r\n':
            string = GetStringFromBytes(ser.readline())
            final += string
        #final = '@values\r\n1\r\n10\r\nabcdefghijklmnopqrstuvxyzabcdef\r\nSensorBoard\r\nLab\r\n192.168.1.1\r\n9000\r\n/upload/wavesegment\r\nmtnl.net\r\nmtnl\r\nmtnl123\r\n0.0.0.0\r\n0.0.0.0\r\n0.0.0.0\r\n@end-values\r\n'
        print(final)    
        ser.close()
        
        PutValues(final)

    except serial.SerialException:
        StatusLabel.set("Serial Port:'" + COMPort + "' Not Available")
        print(StatusLabel.get())

def ReturnValues(n):
    if n == -1: return '@values\r\n'
    elif n == 0:  return gSPeriod.get()
    elif n == 1:  return gPPeriod.get()
    elif n == 2:  return gAPIKey.get()
    elif n == 3:  return gDeviceName.get()
    elif n == 4:  return gLocation.get()
    elif n == 5:  return gServerIP.get()
    elif n == 6:  return gServerPort.get()
    elif n == 7:  return gServerURL.get()
    elif n == 8:  return gAPN.get()
    elif n == 9:  return gLogin.get()
    elif n == 10:  return gPassword.get()
    elif n == 11:  return gIPAddress.get()
    elif n == 12:  return gDNS1.get()
    elif n == 13:  return gDNS2.get()
    elif n == 14: return '@end-values\r\n'

def Update():
    try:
        ser = serial.Serial(COMPort, baudrate=19200)
        print(ser)

        StatusLabel.set("Serial Port:'" + COMPort + "' Open")
        print(StatusLabel.get())

        ser.write(bytes('@update\r\n', 'utf-8'))

        string = ''
        while string != '@sendOK\r\n':
            while ser.inWaiting == 0:
               continue

            string = GetStringFromBytes(ser.readline())
            print(string)

        arg=0;

        while arg<15:
            print(ReturnValues(arg))
            ser.write(bytes(ReturnValues(arg), 'utf-8'))
            arg+=1
            string = ''
            while string != '#OK#\r\n':
                while ser.inWaiting == 0:
                   continue
                string = GetStringFromBytes(ser.readline())
                print(string)
                
        while string != '@recvOK\r\n':
            string = GetStringFromBytes(ser.readline())
            print(string)
        
        ser.close()
    except serial.SerialException:
        StatusLabel.set("Serial Port:'" + COMPort + "' Not Available")
        print(StatusLabel.get())
        Initialize()

RootWindow = Tk()

RootWindow.resizable(width=False, height=False)
RootWindow.title('Flyport GSM Module Configurator')

MainFrame1 = Frame(RootWindow, padx=10, pady=10)

StatusFrame = Frame(RootWindow)
StatusFrame.grid(row=1)

LabelEnterDetails1 = Label(MainFrame1, text='Enter Following Details -', padx=10,pady=10)

LabelCOMPort = Label(MainFrame1, text='Serial Port', padx=10,pady=10)

gLabelCOMPort = Entry(MainFrame1, textvariable='Enter the Serial Port', width=50)

gLabelCOMPort.insert(0, 'COM5')

ConnectButton = Button(MainFrame1, text='CONNECT', command=Connect)

StatusLabel = StringVar()
StatusBar = Label(StatusFrame, textvariable=StatusLabel, relief=GROOVE, width = 70)
StatusLabel.set('Enter Port Name')
StatusBar.pack();

MainFrame2 = Frame(RootWindow, padx=10, pady=10)
LabelEnterDetails2 = Label(MainFrame2, text='Enter Following Details -', padx=10,pady=10)
SPeriod = Label(MainFrame2, text='Sampling Period', padx=10,pady=10)
gSPeriod = Entry(MainFrame2, textvariable='Enter the Sampling Period', width=50)
PPeriod = Label(MainFrame2, text='Publish Period', padx=10,pady=10)
gPPeriod = Entry(MainFrame2, textvariable='Enter Publish Period', width=50)
APIKey = Label(MainFrame2, text='API Key', padx=10,pady=10)
gAPIKey = Entry(MainFrame2, textvariable='Enter API Key', width=50)
DeviceName = Label(MainFrame2, text='Device Name', padx=10,pady=10)
gDeviceName = Entry(MainFrame2, textvariable='Enter Device Name', width=50)
Location = Label(MainFrame2, text='Location', padx=10,pady=10)
gLocation = Entry(MainFrame2, textvariable='Enter Location', width=50)
ServerIP = Label(MainFrame2, text='Server IP', padx=10,pady=10)
gServerIP = Entry(MainFrame2, textvariable='Enter ServerIP', width=50)
ServerPort = Label(MainFrame2, text='Server Port', padx=10,pady=10)
gServerPort = Entry(MainFrame2, textvariable='Enter ServerPort', width=50)
ServerURL = Label(MainFrame2, text='Server URL', padx=10,pady=10)
gServerURL = Entry(MainFrame2, textvariable='Enter ServerURL', width=50)
APN = Label(MainFrame2, text='APN', padx=10,pady=10)
gAPN = Entry(MainFrame2, textvariable='Enter APN', width=50)
Login = Label(MainFrame2, text='AP Login', padx=10,pady=10)
gLogin = Entry(MainFrame2, textvariable='Enter Login', width=50)
Password = Label(MainFrame2, text='AP Password', padx=10,pady=10)
gPassword = Entry(MainFrame2, textvariable='Enter Password', width=50)
IPAddress = Label(MainFrame2, text='IP Address', padx=10,pady=10)
gIPAddress = Entry(MainFrame2, textvariable='Enter IPAddress', width=50)
DNS1 = Label(MainFrame2, text='DNS1', padx=10,pady=10)
gDNS1 = Entry(MainFrame2, textvariable='Enter DNS1', width=50)
DNS2 = Label(MainFrame2, text='DNS2', padx=10,pady=10)
gDNS2 = Entry(MainFrame2, textvariable='Enter DNS2', width=50)
UpdateButton = Button(MainFrame2, text='UPDATE', command=Update)

MainFrame1.grid(row=0)
Initialize()

RootWindow.mainloop()

i=0

print(i)
