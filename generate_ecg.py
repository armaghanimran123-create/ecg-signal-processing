import numpy as np
import matplotlib.pyplot as plt
from scipy import signal

# 1. Simulate a "Heartbeat" (Synthetic QRS Complex)
fs = 500  # Sampling Frequency (500 Hz)
t = np.linspace(0, 3, 3 * fs, endpoint=False)  # 3 seconds of data

# Create a periodic "pulse" (simulating a heartbeat)
ecg_clean = signal.gausspulse(np.mod(t, 1) - 0.2, fc=5) 

# 2. Add the "Enemy": 50Hz Power Line Interference + Random Noise
power_interference = 0.5 * np.sin(2 * np.pi * 50 * t) # 50Hz hum
random_noise = 0.2 * np.random.randn(len(t))
noisy_ecg = ecg_clean + power_interference + random_noise

# 3. Apply the "Solution": Notch Filter (Removes 50Hz) + Lowpass
# Filter out exactly 50Hz
b_notch, a_notch = signal.iirnotch(50, 30, fs)
filtered_ecg = signal.filtfilt(b_notch, a_notch, noisy_ecg)

# Filter out high freq noise (Lowpass at 100Hz)
b_low, a_low = signal.butter(4, 100 / (0.5 * fs), btype='low')
final_ecg = signal.filtfilt(b_low, a_low, filtered_ecg)

# 4. Plot and Save
plt.figure(figsize=(10, 5))

# Top: Dirty Signal
plt.subplot(2, 1, 1)
plt.plot(t[:500], noisy_ecg[:500], color='#e74c3c', linewidth=1) # Zoom in on 1st second
plt.title('Raw ECG Signal (Corrupted by 50Hz Power Line Noise)', fontweight='bold')
plt.grid(True, alpha=0.3)
plt.ylabel('Amplitude (mV)')

# Bottom: Clean Signal
plt.subplot(2, 1, 2)
plt.plot(t[:500], final_ecg[:500], color='#2ecc71', linewidth=1.5)
plt.title('Filtered ECG Output (Clean QRS Complex)', fontweight='bold')
plt.grid(True, alpha=0.3)
plt.xlabel('Time (seconds)')
plt.ylabel('Amplitude (mV)')

plt.tight_layout()
plt.savefig('ecg_results.png', dpi=300)
print("✅ Success! Saved 'ecg_results.png'")
plt.show()