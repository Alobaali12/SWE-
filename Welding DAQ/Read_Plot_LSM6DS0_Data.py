import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

plt.close('all')
filename = 'Raw Data Test1/5/1.Welding_Test8.csv'
data = pd.read_csv(filename)

Time = np.array(data.Time) #[h:m:s.millis]
Distance = np.array(data.CTWD) # [mm]
Working_Angle = np.array(data.WorkingAngle) # [°]
Travel_Angle = np.array(data.TravelAngle) # [°]

Working_Angle = 180-Working_Angle
prom = np.mean(Working_Angle)
aux = Working_Angle-prom
Working_Angle = prom - aux - 1.2

plt.figure(1)
plt.plot(Time, Distance*0.15052540514959561, label="raw data")
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
plt.plot(Time, Working_Angle , label="raw data")
plt.ylim(40,55)
plt.axhline(y = 42, color = 'y', linestyle = '--',label="acceptable limits")
plt.axhline(y = 48, color = 'y', linestyle = '--')
plt.axhline(y = 53, color = 'r', linestyle = '--',label="out of limits")
plt.axhline(y = 37, color = 'r', linestyle = '--')
plt.legend()
plt.xlabel('Time [s]')
plt.ylabel("Working Angle [°]")
plt.title("Working Angle vs Time")

plt.figure(3)
plt.plot(Time, Travel_Angle+8.5, label="raw data")
plt.ylim(-5,20)
plt.axhline(y = 5, color = 'y', linestyle = '--',label="acceptable limits")
plt.axhline(y = 15, color = 'y', linestyle = '--')
plt.axhline(y = 18, color = 'r', linestyle = '--',label="out of limits")
plt.axhline(y = 2, color = 'r', linestyle = '--')
plt.legend()
plt.xlabel('Time [s]')
plt.ylabel("Travel Angle [°]")
plt.title("Travel Angle vs Time")