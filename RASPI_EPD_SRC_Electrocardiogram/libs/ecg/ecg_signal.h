//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   ecg_signal.h
//          Description         :   ECG signal generator for demo
//          License             :   GNU 
//          Author              :   Lio
//          Hardware            :   Raspberry Pi Zero 2W + e-Paper 2.66"
//     
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <cmath>

class ECGSignal {
public:
    ECGSignal(int screenWidth, int screenHeight);
    
    // Generate next sample and return Y position
    int nextSample();
    
    // Get current heart rate (BPM)
    int getBPM() const { return m_bpm; }
    
    // Set heart rate
    void setBPM(int bpm) { m_bpm = bpm; }
    
    // Reset to beginning of cycle
    void reset() { m_phase = 0; }

private:
    float generateWaveform(float phase);
    
    int m_width;
    int m_height;
    int m_bpm;
    float m_phase;
    float m_phaseIncrement;
    
    // ECG waveform parameters (normalized 0-1)
    static constexpr float P_WAVE_HEIGHT = 0.15f;
    static constexpr float QRS_HEIGHT = 0.85f;
    static constexpr float T_WAVE_HEIGHT = 0.25f;
    static constexpr float BASELINE = 0.5f;
};
