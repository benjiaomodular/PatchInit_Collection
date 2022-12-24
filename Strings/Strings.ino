#include "DaisyDuino.h"

DaisyHardware patch;
Switch button;
Switch toggle;

static StringOsc str;
static Oscillator lfo;
static AdEnv adenv;

float voltsPerNote = 0.0833;
float note = 0;
float freq = 440.0;

float brightness = 0;
float damp = 0;
float nl = 0;

void AudioCallback(float**  in, float** out, size_t size)
{
    float sig_out, trig;
    trig = 0.0f;
    patch.DebounceControls();

    // Handle Triggering the Plucks
    trig = 0.0f;
    if (patch.gateIns[0].Trig())
      trig = 1.0f;

    /** Set the oscillators to selected frequency */
    str.SetFreq(freq);

    /** This loop will allow us to process the individual samples of audio */
    for(size_t i = 0; i < size; i++)
    {
        float lfo_sig = lfo.Process();

        /** In this example both outputs will be the same */
        out[0][i] = out[1][i] = str.Process(trig);

        // Light up LED based on envelope output
        patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_1, 5 * lfo_sig);
        patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_2, 5 * lfo_sig);
    }

}

void setup()
{
    float sample_rate;
    patch = DAISY.init(DAISY_PATCH_SM);
    sample_rate = DAISY.get_samplerate();
    adenv.Init(sample_rate);

    // Set up String algo
    str.Init(sample_rate);
    str.SetDamping(.8f);
    str.SetNonLinearity(.1f);
    str.SetBrightness(.5f);

    // setup lfo at .25Hz
    lfo.Init(sample_rate);
    lfo.SetAmp(1.f);
    lfo.SetFreq(.1f);

    DAISY.StartAudio(AudioCallback);
}

void loop(){
    button.Debounce();
    toggle.Debounce();
    patch.ProcessAnalogControls();

    /** Compute note frequency **/
    float note_v = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_5));
    float coarse_tune = 12.f + (patch.GetAdcValue(0) * 72.f);
    note = int(note_v / voltsPerNote);
    freq = mtof(note + coarse_tune);

    float cv_2 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_2));
    float cv_6 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_6));
    brightness = (cv_2 + cv_6) / 5.0;
    str.SetBrightness(brightness);

    float cv_3 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_3));
    float cv_7 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_7));
    damp = (cv_3 + cv_7) / 5.0;
    str.SetDamping(damp);

    float cv_4 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_4));
    float cv_8 = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_8));
    nl = (cv_4 + cv_8) / 5.0;
    str.SetNonLinearity(nl);
}
