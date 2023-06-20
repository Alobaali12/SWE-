import serial
import csv

# Define the serial port and baud rate
serial_port = 'COM6'  # Replace with your Arduino's serial port
baud_rate = 115200  # Must match the baud rate configured on the Arduino

# Create a serial object
ser = serial.Serial(serial_port, baud_rate)

# Create a CSV file to write the data
csv_file = open('data.csv', 'w', newline='')
csv_writer = csv.writer(csv_file)
csv_writer.writerow(['Time', 'Pitch', 'Yaw', 'Roll', 'CTWD', 'HD'])

# Read and process the data from the serial port
while True:
    if ser.in_waiting > 0:
        data = ser.readline().decode('utf-8').rstrip()
        values = data.split(', ')
        
        if len(values) == 6:
            time = int(values[0])
            Pitch = float(values[1])
            Yaw = float(values[2])
            Roll = float(values[3])
            CTWD = float(values[4])
            HD = float(values[5])
            csv_writer.writerow([time, Pitch, Yaw, Roll, CTWD, HD])
            csv_file.flush()


