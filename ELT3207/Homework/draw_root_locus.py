import numpy as np
import control
import matplotlib.pyplot as plt

# Define the Open-Loop Transfer Function G(s)
num_poles = [-0.19, -0.01]
numerator = np.poly(num_poles)
den_poles = [-0.4, -0.5, -0.161, -1.539]
denominator = np.poly(den_poles)
denominator = np.convolve(denominator, [1, 0])
sys = control.TransferFunction(numerator, denominator)

# Draw the Root Locus
control.root_locus(sys)

# Display the plot
plt.rcParams.update({
    'font.size': 25,
    'xtick.labelsize': 25,
    'ytick.labelsize': 25
})
ax = plt.gca()
for line in ax.get_lines():
    if line.get_marker() == 'x':
        line.set_markersize(12)
        line.set_markeredgewidth(3)

plt.grid(True)
plt.show()