# Real-Time ECG Signal Processing & Noise Cancellation

## 📌 Overview
This project implements a digital signal processing pipeline to acquire and clean biopotential signals (ECG). It addresses common biomedical noise sources, specifically **50Hz power line interference** and baseline wander, to extract a clean QRS complex for heart rate analysis.

## 🛠️ Tech Stack
- **Core Logic:** C (Embedded implementation for speed and efficiency).
- **Simulation/Analysis:** Python (SciPy, NumPy) for algorithm validation and visualization.
- **Signal Processing:** IIR Notch Filters, Butterworth Low-pass Filters.

## 📊 Results
The system successfully attenuates 50Hz noise while preserving the diagnostic QRS features.
![ECG Signal Filtering Results](ecg_results.png)
*Top: Raw Signal corrupted by power line noise | Bottom: Cleaned Output*

## 🚀 How it Works
1. **Acquisition:** Simulates raw analog ECG input with noise artifacts.
2. **Notch Filtering:** Removes the specific 50Hz hum from mains electricity.
3. **Bandpass Filtering:** Isolates the 0.5Hz - 100Hz frequency band relevant for human heartbeats.
