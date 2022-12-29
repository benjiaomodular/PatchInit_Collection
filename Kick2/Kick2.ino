/** 
 *  Based on the analogbassdrum examples from DaisyDuino: https://github.com/electro-smith/DaisyDuino/blob/master/examples/Seed/analogbassdrum/analogbassdrum.ino
**/

#include "DaisyDuino.h"

DaisyHardware patch;
Switch button;
Switch toggle;

SyntheticBassDrum bd;

float voltsPerNote = 0.0833;
float tn = 0;
float note = 0;
float freq = 100.0f;
float decay = 0.5f;
float fm = 0.5f;

void AudioCallback(float**  in, float** out, size_t size)
{
    float sig_out, trig;
    trig = 0.0f;
    patch.DebounceControls();

    // Handle Triggering the Plucks
    if (patch.gateIns[0].Trig()) trig = 1.0f;

    /** Set the oscillators to selected frequency */

    /** This loop will allow us to process the individual samples of audio */
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = out[1][i] = bd.Process(trig);
        
        // Light up LED based on envelope output
        patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_1, 5 * trig);
        patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_2, 5 * trig);
    }

}

void setup()
{
    float sample_rate;
    patch = DAISY.init(DAISY_PATCH_SM);
    sample_rate = DAISY.get_samplerate();

    bd.Init(sample_rate);
    bd.SetFreq(freq);

    DAISY.StartAudio(AudioCallback);
}

void loop(){
    button.Debounce();
    toggle.Debounce();
    patch.ProcessAnalogControls();

    /** Compute note frequency **/
//    float coarse_tune = patch.GetAdcValue(0) * 72.f;
    float note_v = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_5));
    note = int(note_v / voltsPerNote);
    freq = mtof(note);
    bd.SetFreq(freq);

    float cv_1 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_1));
    float accent = cv_1 / 5.0f;
    bd.SetAccent(accent);

    float cv_2 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_2));
    float cv_6 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_6));
    float dirt = (cv_2 + cv_6) / 5.0f;
    bd.SetDirtiness(dirt);

    float cv_3 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_3));
    float cv_7 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_7));
    float fm_env = (cv_3 + cv_7) / 5.0f;
    bd.SetFmEnvelopeAmount(fm_env);

    float cv_4 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_4));
    float cv_8 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_8));
    float decay = (cv_4 + cv_8) / 5.0f;
    bd.SetFmEnvelopeDecay(decay);
}
