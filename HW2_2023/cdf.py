import matplotlib.pyplot as plt

# Data for Interrupt 0
interrupt0_data = {
    0: 0,
    1: 0,
    2: 0,
    3: 63861,
    4: 9102,
    5: 10070,
    6: 1795,
    7: 2028,
    8: 2206,
    9: 842,
    10: 899,
    11: 231,
    12: 284,
    13: 329,
    14: 132,
    15: 128,
    16: 32,
    17: 37,
    18: 45,
    19: 14,
    20: 34,
    21: 5,
    22: 3,
    23: 8,
    24: 1,
    25: 2,
    26: 0,
    27: 0,
    28: 1,
    29: 0,
    30: 0,
    31: 0,
    32: 0,
    33: 0,
    34: 1
}

# Data for Interrupt 1
interrupt1_data = {
    0: 0,
    1: 0,
    2: 0,
    3: 0,
    4: 0,
    5: 20680,
    6: 3382,
    7: 3634,
    8: 831,
    9: 890
}

# Calculate CDF for Interrupt 0
interrupt0_cdf = {}
total_count_0 = sum(interrupt0_data.values())
cumulative_count_0 = 0
for k, v in sorted(interrupt0_data.items()):
    cumulative_count_0 += v
    interrupt0_cdf[k] = cumulative_count_0 / total_count_0

# Calculate CDF for Interrupt 1
interrupt1_cdf = {}
total_count_1 = sum(interrupt1_data.values())
cumulative_count_1 = 0
for k, v in sorted(interrupt1_data.items()):
    cumulative_count_1 += v
    interrupt1_cdf[k] = cumulative_count_1 / total_count_1

# Plot CDF for Interrupt 0
plt.plot(interrupt0_cdf.keys(), interrupt0_cdf.values(), label='Interrupt 0')

# Plot CDF for Interrupt 1
plt.plot(interrupt1_cdf.keys(), interrupt1_cdf.values(), label='Interrupt 1')

plt.xlabel('Service Cycles')
plt.ylabel('Cumulative Probability')
plt.legend()
plt.grid()
plt.title('Cumulative Distribution Function (CDF) for Interrupts')
plt.show()
