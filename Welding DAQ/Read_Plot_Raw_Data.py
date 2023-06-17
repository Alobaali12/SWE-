import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Make sure to update the data filename
# Also make sure this file is update for the data collected
# This version assumes collection of IMU data from two daisy-chained LSM6DSO devices
# All data may not have been collected, depending on what you did in lab

plt.close('all')
# --------------------------- Simulator --------------------------- #
#file_Simulator = 'Static/2/04-18-2023_08-56.csv'
file_Simulator = '04-10-2023_01-45.csv'
data_Simulator = pd.read_csv(file_Simulator)

Time_Simulator = np.array(data_Simulator.Time) #[h:m:s.millis]
time_Simulator = [0] * len(Time_Simulator) # s.millis
Distance_Simulator = np.array(data_Simulator.CTWD) # [mm]
Working_Angle_Simulator = np.array(data_Simulator.WorkingAngle) # [°]
Travel_Angle_Simulator = np.array(data_Simulator.TravelAngle) # [°]
Speed_Simulator = np.array(data_Simulator.Speed) # [mm/s]

for i in range (len(Time_Simulator)):
    Time_Simulator[i] = Time_Simulator[i].split(":")
    for j in range (len(Time_Simulator[i])):
        Time_Simulator[i][j] = float(Time_Simulator[i][j])
        if j==0:
            Time_Simulator[i][j] = Time_Simulator[i][j]*60*60
        if j==1:
            Time_Simulator[i][j] = Time_Simulator[i][j]*60
        time_Simulator[i] = time_Simulator[i] + Time_Simulator[i][j] 

constant = time_Simulator[0]    
for i in range (len(time_Simulator)):
    time_Simulator[i] = time_Simulator[i]- constant
# --------------------------- Sensors --------------------------- #
file_Sensors = '1.Welding_Test6.csv'
data_Sensors = pd.read_csv(file_Sensors)

Time_Sensors = np.array(data_Sensors.Time) #[h:m:s.millis]
Distance_Sensors = np.array(data_Sensors.CTWD) # [mm]
Working_Angle_Sensors = np.array(data_Sensors.WorkingAngle) # [°]
Travel_Angle_Sensors = np.array(data_Sensors.TravelAngle) # [°]

# Working_Angle_Sensors = 180-Working_Angle_Sensors
# prom = np.mean(Working_Angle_Sensors)
# aux = Working_Angle_Sensors-prom
# Working_Angle_Sensors = prom - aux - 1.2
# --------------------------- Factors --------------------------- #
Dist_Factor = np.mean(Distance_Simulator)-np.mean(Distance_Sensors)
Work_Ang_Factor = np.mean(Working_Angle_Simulator)-np.mean(Working_Angle_Sensors)
Trav_Ang_Factor = np.mean(Travel_Angle_Simulator)-np.mean(Travel_Angle_Sensors)

# ----------------------------- Plot ----------------------------- #
plt.figure(1)
plt.plot(time_Simulator, Distance_Simulator, label="Simulator")
#3 plt.plot(Time_Sensors[210:-1]-Time_Sensors[210], Distance_Sensors[210:-1]*0.5+Dist_Factor*0.5, label="Sensors")
#4 plt.plot(Time_Sensors[550:-1]-Time_Sensors[550], Distance_Sensors[550:-1]*0.5+Dist_Factor*0.5, label="Sensors")
#5 plt.plot(Time_Sensors[286:-1]-Time_Sensors[286], Distance_Sensors[286:-1]*0.7+Dist_Factor*0.7+1.8, label="Sensors")
#S1 plt.plot(Time_Sensors[320:-1]-Time_Sensors[320], Distance_Sensors[320:-1]*0.7+Dist_Factor*0.7+1.4, label="Sensors")
#S2 plt.plot(Time_Sensors[150:-1]-Time_Sensors[150], Distance_Sensors[150:-1]*1+Dist_Factor*1, label="Sensors")
plt.plot(Time_Sensors[150:-1]-Time_Sensors[150], Distance_Sensors[150:-1]*1+Dist_Factor*1, label="Sensors")
# plt.plot(Time_Sensors, Distance_Sensors, label="Sensors")
plt.ylim(-1,10)
plt.axhline(y = 1, color = 'y', linestyle = '--',label="acceptable limits")
plt.axhline(y = 3, color = 'y', linestyle = '--')
plt.axhline(y = 0, color = 'r', linestyle = '--',label="out of limits")
plt.axhline(y = 5, color = 'r', linestyle = '--')
plt.legend()
plt.xlabel('Time [s]')
plt.ylabel("CTWD [mm]")
plt.title("CTWD vs Time")

plt.figure(2)
plt.plot(time_Simulator, Working_Angle_Simulator , label="Simulator")
#3 plt.plot(Time_Sensors[210:-1]-Time_Sensors[210], Working_Angle_Sensors[210:-1]+Work_Ang_Factor , label="Sensors")
#4 plt.plot(Time_Sensors[550:-1]-Time_Sensors[550], Working_Angle_Sensors[550:-1]+Work_Ang_Factor , label="Sensors")
#5 plt.plot(Time_Sensors[286:-1]-Time_Sensors[286], Working_Angle_Sensors[286:-1]+Work_Ang_Factor , label="Sensors")
#S1 plt.plot(Time_Sensors[320:-1]-Time_Sensors[320], Working_Angle_Sensors[320:-1]+Work_Ang_Factor+1 , label="Sensors")
#S2 plt.plot(Time_Sensors[150:-1]-Time_Sensors[150], Working_Angle_Sensors[150:-1]+Work_Ang_Factor , label="Sensors")
plt.plot(Time_Sensors[150:-1]-Time_Sensors[150], Working_Angle_Sensors[150:-1]+Work_Ang_Factor , label="Sensors")
# plt.plot(Time_Sensors, Working_Angle_Sensors, label="Sensors")
plt.ylim(30,58)
plt.axhline(y = 42, color = 'y', linestyle = '--',label="acceptable limits")
plt.axhline(y = 48, color = 'y', linestyle = '--')
plt.axhline(y = 53, color = 'r', linestyle = '--',label="out of limits")
plt.axhline(y = 37, color = 'r', linestyle = '--')
plt.legend()
plt.xlabel('Time [s]')
plt.ylabel("Working Angle [°]")
plt.title("Working Angle vs Time")

plt.figure(3)
plt.plot(time_Simulator, Travel_Angle_Simulator, label="Simulator")
#3 plt.plot(Time_Sensors[210:-1]-Time_Sensors[210], Travel_Angle_Sensors[210:-1]+Trav_Ang_Factor, label="Sensors")
#4 plt.plot(Time_Sensors[550:-1]-Time_Sensors[550], Travel_Angle_Sensors[550:-1]+Trav_Ang_Factor, label="Sensors")
#5 plt.plot(Time_Sensors[286:-1]-Time_Sensors[286], Travel_Angle_Sensors[286:-1]+Trav_Ang_Factor, label="Sensors")
#S1 plt.plot(Time_Sensors[320:-1]-Time_Sensors[320], Travel_Angle_Sensors[320:-1]+Trav_Ang_Factor, label="Sensors")
#S2 plt.plot(Time_Sensors[150:-1]-Time_Sensors[150], Travel_Angle_Sensors[150:-1]+Trav_Ang_Factor, label="Sensors")
plt.plot(Time_Sensors[150:-1]-Time_Sensors[150], Travel_Angle_Sensors[150:-1]+Trav_Ang_Factor*0, label="Sensors")
# plt.plot(Time_Sensors, Travel_Angle_Sensors, label="Sensors")
plt.ylim(-5,25)
plt.axhline(y = 5, color = 'y', linestyle = '--',label="acceptable limits")
plt.axhline(y = 15, color = 'y', linestyle = '--')
plt.axhline(y = 18, color = 'r', linestyle = '--',label="out of limits")
plt.axhline(y = 2, color = 'r', linestyle = '--')
plt.legend()
plt.xlabel('Time [s]')
plt.ylabel("Travel Angle [°]")
plt.title("Travel Angle vs Time")

# plt.figure(4)
# plt.plot(time_Simulator, Speed_Simulator, label="raw data")
# plt.ylim(0,14)
# plt.axhline(y = 10, color = 'y', linestyle = '--',label="acceptable limits")
# plt.axhline(y = 6, color = 'y', linestyle = '--')
# plt.axhline(y = 4, color = 'r', linestyle = '--',label="out of limits")
# plt.axhline(y = 12, color = 'r', linestyle = '--')
# plt.legend()
# plt.xlabel('Time [s]')
# plt.ylabel("Travel Speed [mm/s]")
# plt.title("Travel Speed vs Time")