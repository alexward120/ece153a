import matplotlib.pyplot as plt
import numpy as np

problem = int(input("Enter the problem number: ")) # either 1 or 2
if problem == 1:
    file_name_cache = "cached1.txt"
    file_name_uncached_cache = "uncached1.txt"
elif problem == 2:
    file_name_cache = "cached2.txt"
    file_name_uncached_cache = "uncached2.txt"
else:
    print("Invalid problem number")
    exit()

access_times_enabled_cache = []
access_times_disabled_cache = []

# Read the data from the files
with open(file_name_cache, "r") as file:
    for line in file:
        try:
            num = int(line.strip())
            access_times_enabled_cache.append(num)
        except ValueError:
            print(f'Invalid line: "{line}"')

with open(file_name_uncached_cache, "r") as file:
    for line in file:
        try:
            num = int(line.strip())
            access_times_disabled_cache.append(num)
        except ValueError:
            print(f'Invalid line: "{line}"')

# Sort the data
access_times_disabled_cache.sort()
access_times_enabled_cache.sort()

# Calculate cumulative probabilities
total_data_points = len(access_times_disabled_cache)
cdf_disabled_cache = np.arange(1, total_data_points + 1) / total_data_points

total_data_points = len(access_times_enabled_cache)
cdf_enabled_cache = np.arange(1, total_data_points + 1) / total_data_points

# Plot the CDF
plt.plot(access_times_disabled_cache, cdf_disabled_cache, label="Disabled Cache")
plt.plot(access_times_enabled_cache, cdf_enabled_cache, label="Enabled Cache")

plt.xlabel("Access Time (cycles)")
plt.ylabel("Cumulative Probability")
plt.legend()
plt.grid(True)

plt.show()
