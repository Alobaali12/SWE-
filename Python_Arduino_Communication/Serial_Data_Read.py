import serial
import csv
import datetime

# Define the serial port and baud rate
serial_port = 'COM6'  # Replace with your Arduino's serial port
baud_rate = 9600  # Must match the baud rate configured on the Arduino

# Create a serial object
ser = serial.Serial(serial_port, baud_rate)

# Generate a unique CSV file name based on current timestamp
timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
csv_filename = f"data_{timestamp}.csv"

# Create a CSV file to write the data
csv_file = open(csv_filename, 'w', newline='')
csv_writer = csv.writer(csv_file)
csv_writer.writerow(['Time', 'Q_w', 'Q_x', 'Q_y', 'Q_z', 'Roll', 'Pitch', 'Yaw', 'CTWD', 'HD', 'HV', 'Mic'])

# Read and process the data from the serial port
try:
    while True:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').rstrip()
            values = data.split(', ')
            
            if len(values) == 12:
                time = float(values[0])
                Q_w = float(values[1])
                Q_x = float(values[2])
                Q_y = float(values[3])
                Q_z = float(values[4])
                Pitch = float(values[5])
                Yaw = float(values[6])
                Roll = float(values[7])
                CTWD = float(values[8])
                HD = float(values[9])
                HV = float(values[10])
                Mic = float(values[11])
                
                csv_writer.writerow([time, Q_w, Q_x, Q_y, Q_z, Pitch, Yaw, Roll, CTWD, HD, HV, Mic])
                csv_file.flush()
except KeyboardInterrupt:
    print("Program interrupted. Closing the serial port and CSV file.")

# Close the serial port and CSV file
ser.close()
csv_file.close()
