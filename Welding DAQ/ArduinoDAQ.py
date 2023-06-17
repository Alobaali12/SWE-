import sys
import serial
import time
import struct
import pandas as pd

class SerialConnect:
    """
    __init__:
        initializes the SerialConnect object
        checks to see all data lists are the same length
    """
    def __init__(self, serialPort, fileName, serialBaud, dataRate, \
                 dataNames, dataTypes,commandTimes=[], commandData=[], commandTypes=[]):
        self.port = serialPort
        self.fileName = fileName
        self.baud = serialBaud
        self.dataRate = dataRate  # Hz
        self.period = 1.0/dataRate  # s
        
        # Check to see that the number of data names and types match
        if len(dataNames) != len(dataTypes):
            print("Length of dataNames: " + str(len(dataNames)) + ", does not equal length of dataTypes: " + str(len(dataTypes)) + ".")
            print("Exiting.")
            sys.exit()
        
        self.dataNames = dataNames
        self.dataTypes = dataTypes
        
        # Create data frame of commands to send to Arduino
        command_dict = {'time': commandTimes, 'value': commandData, 'command': commandTypes}
        self.command_df = pd.DataFrame(command_dict)
        self.command_df_idx = 0
        
        # initialize a list of lists to store all of the collected data
        self.dataStore = [[] for _ in range(len(dataNames))]
        
        # Calculate total number of bytes the Arduino is sending over each cycle
        self.totalNumBytes = sum([struct.calcsize(dataType) for dataType in self.dataTypes])
        
        # Used to see how many times the while loop in GetSerialData ran before acquiring all the data
        # For error checking
        self.waitingCounter = 0
        
        # For debug purposes
        self.waitingList = []
      
        
    """
    connectToArduino:
         attepmts to connect to Arduino through the serial port and sends
         the acquistion data rate to the Arduino
    """
    def connectToArduino(self):
        print('Trying to connect to: ' + str(self.port) + ' at ' + str(self.baud) + ' BAUD.')
        try:
            self.serialConnection = serial.Serial(self.port, self.baud, timeout=4)
            print('Connected to ' + str(self.port) + ' at ' + str(self.baud) + ' BAUD.')
        except:
            print("Failed to connect with " + str(self.port) + ' at ' + str(self.baud) + ' BAUD.')
            sys.exit()
        
        print("Starting in 3...")            
        time.sleep(1.0)  # give some buffer time for retrieving data
        
        print("2...")
        self.serialConnection.reset_input_buffer()
        time.sleep(1.0)  # give some buffer time for retrieving data
    
        print("1...")
        time.sleep(1.0)
        
        rateStr = "r," + str(self.dataRate) + "\n"
        print(rateStr)
        self.serialConnection.write(rateStr.encode())  # Send over data rate to Arduino


    """
    getSerialData:
        collects and stores all data sent from Arduino in one data-collection cycle
        this function is called once per cycle
    """
    def getSerialData(self):

        # Run infinte loop until 
        while True:
            if self.serialConnection.inWaiting() >= self.totalNumBytes: # Check to all bytes are available     
                for index in range(len(self.dataNames)):
                    self.readVariable(index)
                    
                self.waitingList.append(self.waitingCounter)
                self.waitingCounter = 0  # reset the counter
                break  # break out of the infinte while loop, since data has been collected this cycle  
            
            self.waitingCounter += 1  # Increment the wait counter since data has not been collected yet
            
            # Wait an arbitrarily long time to check if data is being sent over
            # If wait counter is too large, assume something is wrong and exit
            if self.waitingCounter > 1000000:
                print("We've been waiting for a while now. The Arduino is not sending over enough data.")
                print("Check your Arduino code or sensor hookup.")
                print("Exiting.")
                self.serialConnection.close()
                sys.exit()

    """
    readVariable:
        reads and store single varible sent over from Arduino based on its type
        this function is called for each variable every data-collection cycle
    """
    def readVariable(self, index):
        dataType = self.dataTypes[index]
        numBytes = struct.calcsize(dataType)
        
        varString = self.serialConnection.read(numBytes)  # read corresponding number of bytes into a bytestring
        var, = struct.unpack(dataType, varString)  # unpack bytestring into a variable (i.e. a number)
        
        # Convert time data from microseconds to seconds
        if index == 0:
            var = var*1e-6
        
        self.dataStore[index].append(var)  # append the variable to the corresponding list in the dataStore

    """
    sendCommand:
        this function
    """
    def sendCommand(self):
        if self.command_df_idx < len(self.command_df):
            if (len(self.dataStore[0]) /self.dataRate >= self.command_df.time[self.command_df_idx]):
                commandStr = self.command_df.command[self.command_df_idx] + "," + str(self.command_df.value[self.command_df_idx]) + '\n'
                self.serialConnection.write(commandStr.encode())  # Send pwm command to Arduino
                self.command_df_idx = self.command_df_idx + 1
                print(commandStr)

    """
    close:
        serial connection and save data to file
    """
    def close(self):

        self.serialConnection.close()
        print('Disconnected...')
        
        df = pd.DataFrame(self.dataStore).transpose()
        df.columns = self.dataNames

        # Start time at zero seconds
        if 'Time' in df.columns:
            df.Time = df.Time - df.Time.iloc[0]

        # Save to file
        df.to_csv(self.fileName, index=False)
