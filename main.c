#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ecg_processor.h"

// Note: Ensure synthetic_ecg_signal.csv is in the same folder or provide full path
#define CSV_FILENAME "data/synthetic_ecg_signal_generated.csv" 

int main() {
    printf("--- EMBEDDED ECG PROCESSOR SIMULATION ---\n");
    printf("Initializing Processor...\n");
    
    ECG_Context ecg;
    ECG_Init(&ecg);
    
    FILE *fp = fopen(CSV_FILENAME, "r");
    if (!fp) {
        printf("ERROR: Could not open %s\n", CSV_FILENAME);
        printf("Make sure you run the MATLAB script first to generate the data!\n");
        return 1;
    }
    
    char line[1024];
    float t, raw_signal;
    int samples = 0;
    int beats = 0;
    
    printf("Processing Stream...\n");
    
    // Skip header if exists (check your CSV structure)
    // fgets(line, 1024, fp); 
    
    while (fgets(line, 1024, fp)) {
        // Parse CSV: Time, Value
        // Assuming format is: 0.002778,0.1234
        if (sscanf(line, "%f,%f", &t, &raw_signal) != 2) continue;
        
        float filtered_val;
        bool is_beat = ECG_Process_Sample(&ecg, raw_signal, &filtered_val);
        
        if (is_beat) {
            beats++;
            // Calculate instant BPM based on time t
            // (In a real system we calculate delta T between beats)
            printf("[TIME: %.3f s] >> BEAT DETECTED! (Total: %d) | Signal Strength: %.3f\n", 
                   t, beats, filtered_val);
        }
        
        samples++;
    }
    
    fclose(fp);
    printf("--- SIMULATION COMPLETE ---\n");
    printf("Total Samples Processed: %d\n", samples);
    printf("Total Beats Detected: %d\n", beats);
    
    return 0;
}