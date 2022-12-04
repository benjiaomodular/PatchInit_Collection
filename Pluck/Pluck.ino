#include "DaisyDuino.h"

DaisyHardware patch;
static Pluck plk;
float init_buff[256];

float voltsPerNote = 0.0833;
float note = 0;
float damp = 0;
float amp = 0;
float decay = 0.5;
float freq = 440.0f;
float trig;

void AudioCallback(float**  in, float** out, size_t size)
{

    float sig_out, trig;

    trig = 0.0f;
    
    patch.DebounceControls();
    
    // Handle Triggering the Plucks
    trig = 0.0f;
    if (patch.encoder.RisingEdge() or patch.gateIns[0].Trig())
      trig = 1.0f;

    /** This loop will allow us to process the individual samples of audio */
    for(size_t i = 0; i < size; i++)
    {         
    
        plk.SetFreq(freq);

        sig_out = plk.Process(trig);
        out[0][i] = out[1][i] = sig_out;

        // Light up LED based on envelope output
        patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_1, 5 * trig);
        patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_2, 5 * trig);
    }

}

void setup()
{
    patch = DAISY.init(DAISY_PATCH_SM);
    plk.Init(DAISY.AudioSampleRate(), init_buff, 256, PLUCK_MODE_RECURSIVE);

    button.Init(1000, true, PIN_PATCH_SM_B7, INPUT_PULLUP);
    toggle.Init(1000, true, PIN_PATCH_SM_B8, INPUT_PULLUP);
    DAISY.begin(AudioCallback);
}

void loop(){
    patch.ProcessAnalogControls();

    /** Set voice amplitude **/
    float amp_pot = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_2));
    amp = amp_pot / 5.0;
    plk.SetAmp(amp);
        
    /** Set dampening of plucks **/
    float damp_pot = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_3));
    float damp_cv = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_6));
    damp = (damp_pot + damp_cv) / 5.0;
    plk.SetDamp(damp);

    /** Set envelope's decay value **/
    float dec_pot = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_4));
    float dec_cv = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_7));
    decay = 0.0001 + (dec_pot + dec_cv) / 5.0;
    plk.SetDecay(decay);

    /** Compute note frequency **/
    float note_v = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_5));
    float coarse_tune = patch.GetAdcValue(0) * 72.f;
    note = int(note_v / voltsPerNote);
    freq = mtof(note + coarse_tune);

}
