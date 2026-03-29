//
// Created by alisdair chauvin on 12/2/25.
//

#ifndef EFFEM_UNIT_SYNTHSOUND_H
#define EFFEM_UNIT_SYNTHSOUND_H


#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};


#endif //EFFEM_UNIT_SYNTHSOUND_H