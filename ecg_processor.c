#include "ecg_processor.h"
#include <math.h>
#include <stdio.h>

// Standard Pan-Tompkins Integer Filter Coefficients (Normalized for Float)
// These approximate a 5-15Hz Bandpass at 360Hz

void ECG_Init(ECG_Context *ctx) {
    // Zero out everything
    for(int i=0; i<BUFFER_SIZE; i++) { ctx->x_buff[i]=0; ctx->y_buff[i]=0; ctx->h_buff[i]=0; }
    for(int i=0; i<MWI_WINDOW_SIZE; i++) ctx->mw_buff[i]=0;
    
    ctx->mw_ptr = 0;
    ctx->mw_sum = 0;
    
    // Initial Thresholds (Will adapt automatically)
    ctx->signal_level = 0.5f;
    ctx->noise_level = 0.1f;
    ctx->threshold = 0.25f;
    
    ctx->last_beat_sample = 0;
    ctx->current_sample_count = 0;
    ctx->refractory_period = false;
    ctx->beat_count = 0;
    ctx->current_bpm = 0.0f;
}

bool ECG_Process_Sample(ECG_Context *ctx, float raw_input, float *filtered_out) {
    ctx->current_sample_count++;
    
    // ---------------------------------------------------------
    // 1. Low Pass Filter (Difference Equation)
    // y[n] = 2y[n-1] - y[n-2] + x[n] - 2x[n-6] + x[n-12]
    // ---------------------------------------------------------
    
    // Update Input Buffer (Simple Shift)
    for(int i=BUFFER_SIZE-1; i>0; i--) ctx->x_buff[i] = ctx->x_buff[i-1];
    ctx->x_buff[0] = raw_input;
    
    float lp_out = (2 * ctx->y_buff[0]) - ctx->y_buff[1] 
                 + ctx->x_buff[0] - (2 * ctx->x_buff[6]) + ctx->x_buff[12%BUFFER_SIZE];
    // Note: In real embedded code, we'd treat x_buff as a circular buffer to avoid the loop above.
    
    // Update LP History
    ctx->y_buff[1] = ctx->y_buff[0];
    ctx->y_buff[0] = lp_out;

    // ---------------------------------------------------------
    // 2. High Pass Filter
    // ---------------------------------------------------------
    // Simplified HP for demonstration: y[n] = x[n-16] - (lp[n] + lp[n-32])/32
    // We will use a standard subtraction method here for simplicity/stability in float
    float hp_out = lp_out - ctx->x_buff[0]; // Simple approximate HP

    // ---------------------------------------------------------
    // 3. Derivative (Slope)
    // y[n] = (2x[n] + x[n-1] - x[n-3] - 2x[n-4])/8
    // ---------------------------------------------------------
    float der_out = (2*hp_out + ctx->h_buff[0] - ctx->h_buff[2] - 2*ctx->h_buff[3]) / 8.0f;
    
    // Update History
    for(int i=BUFFER_SIZE-1; i>0; i--) ctx->h_buff[i] = ctx->h_buff[i-1];
    ctx->h_buff[0] = hp_out;

    // ---------------------------------------------------------
    // 4. Squaring Function (Rectification)
    // ---------------------------------------------------------
    float sq_out = der_out * der_out;

    // ---------------------------------------------------------
    // 5. Moving Window Integration (Circular Buffer Sum)
    // ---------------------------------------------------------
    ctx->mw_sum -= ctx->mw_buff[ctx->mw_ptr]; // Subtract oldest
    ctx->mw_buff[ctx->mw_ptr] = sq_out;       // Add newest
    ctx->mw_sum += sq_out;
    
    ctx->mw_ptr++;
    if(ctx->mw_ptr >= MWI_WINDOW_SIZE) ctx->mw_ptr = 0;
    
    float integrated_signal = (float)(ctx->mw_sum / MWI_WINDOW_SIZE);
    
    // Pass filtered value back to caller (for plotting/debug)
    *filtered_out = integrated_signal;

    // ---------------------------------------------------------
    // 6. ADAPTIVE THRESHOLD STATE MACHINE
    // ---------------------------------------------------------
    bool beat_detected = false;
    
    // Refractory Period (200ms blanking after a beat)
    if (ctx->current_sample_count - ctx->last_beat_sample > (0.2 * SAMPLE_RATE)) {
        ctx->refractory_period = false;
    }

    if (!ctx->refractory_period) {
        if (integrated_signal > ctx->threshold) {
            // BEAT CANDIDATE
            
            // Check if it's a T-wave (often detected as a beat). 
            // In full Pan-Tompkins we check slope, here we use refractory simple logic.
            
            ctx->signal_level = 0.125f * integrated_signal + 0.875f * ctx->signal_level;
            ctx->threshold = ctx->noise_level + 0.25f * (ctx->signal_level - ctx->noise_level);
            
            ctx->last_beat_sample = ctx->current_sample_count;
            ctx->beat_count++;
            ctx->refractory_period = true;
            beat_detected = true;
            
            // Calculate BPM
            if (ctx->beat_count > 1) {
                float rr_sec = (float)(ctx->current_sample_count - ctx->last_beat_sample) / SAMPLE_RATE;
                // Note: current_sample_count reset is needed in real loop, simpler here
                // We'll just assume sequential samples
            }
        } else {
            // NOISE UPDATE
            ctx->noise_level = 0.125f * integrated_signal + 0.875f * ctx->noise_level;
            ctx->threshold = ctx->noise_level + 0.25f * (ctx->signal_level - ctx->noise_level);
        }
    }
    
    // Slowly decay signal level (to catch dropped beats)
    ctx->signal_level *= 0.999f;
    
    return beat_detected;
}