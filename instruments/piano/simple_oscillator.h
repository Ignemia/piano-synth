/**
 * @file simple_oscillator.h
 * @brief [AI GENERATED] Interface to create the simple oscillator instrument
 *        with subtle frequency variation.
 */

#pragma once

#include "../../shared/interfaces/dll_interfaces.h"

extern "C" {
    /**
     * @brief Create a new simple oscillator instrument instance.
     */
    PianoSynth::Interfaces::IInstrumentSynthesizer* create_instrument_synthesizer();

    /**
     * @brief Destroy a previously created oscillator instrument instance.
     */
    void destroy_instrument_synthesizer(PianoSynth::Interfaces::IInstrumentSynthesizer* synth);
}
