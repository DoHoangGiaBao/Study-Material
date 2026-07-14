import numpy as np
import control as ctrl
import matplotlib.pyplot as plt

T = 1/3                                         # Sampling Time
s = ctrl.TransferFunction.s                     # Laplace variable
K = 0.205 / 0.35                                # Amplifier

# Transfer function (Continuous)
G_ps1 = 1 / (s + 0.4)                           # Actuator and valve
G_ps2 = 0.7 / (s**2 + 1.7 * s + 0.25)           # Chemical heat process
H_s  = 0.5 / (s + 0.5)                          # Temperature sensor
G_cs = ((s + 0.19) * (s + 0.01)) / s            # PID controller

G_s = G_cs * K * G_ps1 * G_ps2                  # Forward Path

T_s = G_s / (1 + G_s * H_s)                     # Closed loop Transfer function

# Digitalized using Tustin method
T_z = ctrl.sample_system(T_s, T, method='bilinear')

# Step response of the system
time, response = ctrl.step_response(T_z)

# Analog response
time_analog = np.linspace(0, 20, 1000)
_, response_analog = ctrl.step_response(T_s, T=time_analog)

# Plot the system response
plt.figure(figsize=(8, 5))

plt.step(time, response, where='post', label='Digital', color='b', linewidth=2)
plt.plot(time_analog, response_analog, label='Analog', color='r', linestyle='--', linewidth=2)

plt.title('Discrete-Time System Step Response (T = 1/3 s)')
plt.ylim([0, 0.9])
plt.xlim([0, 20])
plt.xlabel('Time (s)')
plt.legend()
plt.show()