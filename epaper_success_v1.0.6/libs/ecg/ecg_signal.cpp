//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   ecg_signal.cpp
//          Description         :   ECG signal generator implementation
//          License             :   GNU 
//          Author              :   Lio
//          Hardware            :   Raspberry Pi Zero 2W + e-Paper 2.66"
//     
//////////////////////////////////////////////////////////////////////////////

#include "ecg_signal.h"

ECGSignal::ECGSignal(int screenWidth, int screenHeight)
    : m_width(screenWidth)
    , m_height(screenHeight)
    , m_bpm(72)
    , m_phase(0.0f)
    , m_phaseIncrement(0.0f)
{
    // Sample rate: 100 Hz (10ms per sample)
    // One beat = 60/bpm seconds
    // Phase goes from 0 to 2*PI per beat
    m_phaseIncrement = (2.0f * M_PI * 100.0f) / (m_bpm * 60.0f);
}

int ECGSignal::nextSample() {
    float y = generateWaveform(m_phase);
    
    // Map 0-1 to screen height (with margin)
    int margin = 20;
    int usableHeight = m_height - 2 * margin;
    int yPos = m_height - margin - (int)(y * usableHeight);
    
    // Advance phase
    m_phase += m_phaseIncrement;
    if (m_phase >= 2.0f * M_PI) {
        m_phase -= 2.0f * M_PI;
    }
    
    return yPos;
}

float ECGSignal::generateWaveform(float phase) {
    float value = BASELINE;
    
    // P wave (small bump before QRS)
    // Phase 0.0 - 0.4 (before QRS)
    if (phase >= 0.0f && phase < 0.4f * M_PI) {
        float pPhase = phase / (0.4f * M_PI);
        value += P_WAVE_HEIGHT * sinf(pPhase * M_PI);
    }
    // QRS complex (sharp spike)
    // Phase 0.4 - 0.7 (QRS)
    else if (phase >= 0.4f * M_PI && phase < 0.7f * M_PI) {
        float qrsPhase = (phase - 0.4f * M_PI) / (0.3f * M_PI);
        
        // Q dip (small downward)
        if (qrsPhase < 0.2f) {
            value -= 0.15f * sinf(qrsPhase / 0.2f * M_PI);
        }
        // R peak (sharp upward)
        else if (qrsPhase < 0.5f) {
            float rPhase = (qrsPhase - 0.2f) / 0.3f;
            value += QRS_HEIGHT * sinf(rPhase * M_PI);
        }
        // S dip (small downward)
        else if (qrsPhase < 0.7f) {
            float sPhase = (qrsPhase - 0.5f) / 0.2f;
            value -= 0.2f * sinf(sPhase * M_PI);
        }
        // Return to baseline
        else {
            // Already at baseline
        }
    }
    // ST segment (flat)
    // Phase 0.7 - 1.0
    else if (phase >= 0.7f * M_PI && phase < 1.0f * M_PI) {
        // Flat at baseline
    }
    // T wave (broad bump)
    // Phase 1.0 - 1.6
    else if (phase >= 1.0f * M_PI && phase < 1.6f * M_PI) {
        float tPhase = (phase - 1.0f * M_PI) / (0.6f * M_PI);
        value += T_WAVE_HEIGHT * sinf(tPhase * M_PI);
    }
    // Baseline (rest)
    // Phase 1.6 - 2.0
    else {
        // Flat at baseline
    }
    
    // Clamp to 0-1
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    
    return value;
}
