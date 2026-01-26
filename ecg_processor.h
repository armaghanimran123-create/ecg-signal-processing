#ifndef ECG_PROCESSOR_H
#define ECG_PROCESSOR_H

#include <stdint.h>
#include <stdbool.h>

// Configuration Constants
#define SAMPLE_RATE     360
#define MWI_WINDOW_SIZE 40   // ~150ms window at 360Hz
#define BUFFER_SIZE     10   // For filter history

typedef struct {
    // --- Filter History Buffers (Previous Inputs/Outputs) ---
    float x_buff[BUFFER_SIZE]; // Raw input history
    float y_buff[BUFFER_SIZE]; // Lowpass history
    float h_buff[BUFFER_SIZE]; // Highpass history
    
    // --- Moving Window Integration (MWI) ---
    float mw_buff[MWI_WINDOW_SIZE];
    int mw_ptr;
    double mw_sum; // Double to prevent overflow
    
    // --- Adaptive Threshold State ---
    float signal_level;
    float noise_level;
    float threshold;
    
    // --- Detection Logic ---
    int last_beat_sample;
    int current_sample_count;
    bool refractory_period; // Prevents double-counting a beat
    
    // --- Output Stats ---
    float current_bpm;
    float rr_interval_sum;
    int beat_count;
    
} ECG_Context;

// Function Prototypes
void ECG_Init(ECG_Context *ctx);
bool ECG_Process_Sample(ECG_Context *ctx, float raw_input, float *filtered_out);

#endif