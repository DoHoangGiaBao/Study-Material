import numpy as np
import control as ctrl
import matplotlib.pyplot as plt

T = 1/3 # Sampling time
s = ctrl.TransferFunction.s # Laplace variable
K  = 0.5857 # Amplified coefficient

# Transfer functions (Continuous)
G_plant_s = (1 / (s + 0.4) * (0.7 / (s * s + 1.7 * s + 0.25))) # Plant transfer function
H_sensor_s = 0.5 / (s + 0.5) # Sensor transfer function
G_pid_s = K * (s + 0.19) * (s + 0.01) / s # PID transfer function

# Transfer functions (Discreted)
G_plant_z = ctrl.sample_system(G_plant_s, T, method='zoh') # Plant transfer function
H_sensor_z = ctrl.sample_system(H_sensor_s, T, method='zoh') # Sensor transfer function
G_pid_z = ctrl.TransferFunction(
    [3.632, -7.028, 3.397],
    [1, 0, -1],
    T
) # PID transfer function

# Closed loop
closed_loop_s = ctrl.feedback(G_pid_s * G_plant_s, H_sensor_s)
closed_loop_z = ctrl.feedback(G_pid_z * G_plant_z, H_sensor_z)

# Analog
t_analog = np.linspace(0, 25, 1000)
t_out_analog, y_out_analog = ctrl.step_response(closed_loop_s, t_analog)

# Digital
t_discrete = np.arange(0, 25, T)
t_out_digital, y_out_digital = ctrl.step_response(closed_loop_z, t_discrete)

# Plot
plt.figure(figsize=(10, 6))

plt.plot(t_out_analog, y_out_analog, 'r:', label='Analog', linewidth=1.5)

plt.step(t_out_digital, y_out_digital, where='post', label='Digital', linewidth=2)

plt.ylim([0, 0.9])
plt.xlim([0, 20])
plt.xlabel('time(s)')
plt.legend()
plt.show()
