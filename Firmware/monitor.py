import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import numpy as np
import serial

plt.style.use('dark_background')

port = '/dev/ttyUSB0' #Arduino serial port
baudrate = 9600
timeout = 10 #specify timeout when using readline()
ser = serial.Serial(port, baudrate=baudrate, timeout=timeout)
#ser.open()
if ser.is_open==True:
	print("\nAll right, serial port now open. Configuration:\n")
	print(ser, "\n") #print serial parameters

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
ax.set_ylim((-2**15),(2**15)-1)

samples = 20


xs = np.arange(samples)
acc_array = np.zeros(samples)
acc_sum_array = np.zeros(samples)

dict_axis = {
     0: "X",
     1: "Y",
    2: "Z",
}

acc, acc_sum, add, axis  = [0,0,0,0]

# This function is called periodically from FuncAnimation
def animate(i):
    global acc_array, acc_sum_array, acc, acc_sum, add, axis
    try:
        #Aquire and parse data from serial port
        #line=ser.readline().decode().strip().split(',')      #ascii
        line = ser.read_all()
        if len(line) < 8:
             return
        aux = line.decode().split('\r\n')[-2].split(',')
        acc, acc_sum, add, axis = (int(v) for v in aux)
        print(acc, acc_sum, add, dict_axis[axis])
        
        acc_array = np.roll(acc_array, 1)
        acc_sum_array = np.roll(acc_sum_array, 1)
        
        acc_array[0] = acc
        acc_sum_array[0] = acc_sum/50

        # Draw x and y lists
        ax.clear()
        ax.plot(xs, acc_array, label=f"ACC {dict_axis[axis]}")
        ax.plot(xs, acc_sum_array, label="AVG")
        ax.set_ylim(15000,20000)

        plt.legend()
        #plt.axis([1, None, 0, 1.1]) #Use for arbitrary number of trials
        #plt.axis([1, 100, 0, 1.1]) #Use for 100 trial demo
    except Exception as e:
         print(e)
    

# Set up plot to call animate() function periodically
ani = animation.FuncAnimation(fig, animate, interval=60,repeat=False)
plt.show()