import numpy as np
import matplotlib.pyplot as plt

# Function to generate a sampled sine wave
def generate_sampled_signal(input_frequency, sampling_frequency, num_samples):
    time = np.arange(0, num_samples) / sampling_frequency
    signal = np.sin(2 * np.pi * input_frequency * time)
    return time, signal

# Given input frequencies
input_frequencies = [250, 333, 667, 750, 1250, 1333]
sampling_frequency = 1000  # 1 kHz
num_samples = 50

# Plotting separate graphs for each input frequency
for frequency in input_frequencies:
    time, signal = generate_sampled_signal(frequency, sampling_frequency, num_samples)
    
    plt.figure(figsize=(10, 3))
    plt.plot(time, signal)
    
    plt.title(f'Sampled Signal - Input Frequency: {frequency} Hz')
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.show()
    
    print(f"Actual Frequency: {frequency} Hz")
    
    # Find time difference between consecutive peaks (or zero-crossings)
    peak_indices = np.where((signal[:-1] > 0) & (signal[1:] <= 0))[0]
    time_difference = time[peak_indices[1]] - time[peak_indices[0]]
    
    # Calculate the reconstructed frequency
    reconstructed_frequency = 1 / time_difference
    print(f"Reconstructed Frequency: {reconstructed_frequency} Hz\n")
