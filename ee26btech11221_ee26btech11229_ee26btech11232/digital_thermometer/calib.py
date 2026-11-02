import numpy as np

# raw pt100 readings
x_ours = [
    56.05, 60.63, 61.92, 64.55, 67.18, 
    69.69, 73.90, 76.57, 73.90, 71.18, 
    76.84, 77.93, 78.02, 78.27, 79.31, 
    79.31, 79.31
]

# actual reference thermometer temperatures
y_actual = [
    55.3, 63.9, 64.3, 71.9, 72.9, 
    74.8, 82.9, 84.3, 82.9, 80.8, 
    85.5, 91.2, 92.8, 93.2, 94.2, 
    95.1, 96.8
]

x = np.array(x_ours)
y = np.array(y_actual)

# 1st degree polynomial linear least squares
slope, intercept = np.polyfit(x, y, 1)

print("===== CALIBRATION RESULT =====")
print(f"CAL_A (Slope)     = {slope:.6f}")
print(f"CAL_B (Intercept) = {intercept:.6f}")
print(f"Equation: T_cal = {slope:.6f} * T_raw + {intercept:.6f}")
