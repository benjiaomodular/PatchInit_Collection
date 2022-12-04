#include "DaisyDuino.h"

DaisyHardware patch;
static Oscillator osc;
static AdEnv adenv;
Switch button;
Switch toggle;

float voltsPerNote = 0.0833;
float note = 0;
float attack = 0;
float decay = 0.5;
float freq = 440.0;
bool vca_toggle_state = false;

void AudioCallback(float**  in, float** out, size_t size)
{
    /** Set the oscillators to selected frequency */
    osc.SetFreq(freq);

    /** This loop will allow us to process the individual samples of audio */
    for(size_t i = 0; i < size; i++)
    {       
        float sig = osc.Process(); 
        float env_out = adenv.Process();

        if (vca_toggle_state){
          sig *= env_out;
        }

        /** In this example both outputs will be the same */
        out[0][i] = out[1][i] = sig;

        // Light up LED based on envelope output
        patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_1, 5 * env_out);
        patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_2, 5 * env_out);
    }

}

void setup()
{
    patch = DAISY.init(DAISY_PATCH_SM);
    adenv.Init(DAISY.AudioSampleRate());

    osc.Init(DAISY.AudioSampleRate());
    osc.SetWaveform(Oscillator::WAVE_SIN);

    button.Init(1000, true, PIN_PATCH_SM_B7, INPUT_PULLUP);
    toggle.Init(1000, true, PIN_PATCH_SM_B8, INPUT_PULLUP);

    /* Initialize envelope */
    adenv.SetMin(0.0);
    adenv.SetMax(1);
    adenv.SetCurve(0); // linear

    DAISY.StartAudio(AudioCallback);
}

void loop(){
    button.Debounce();
    toggle.Debounce();
    patch.DebounceControls();

    patch.ProcessAnalogControls();

    /** Compute note frequency **/
    float note_v = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_5));
    float coarse_tune = 12.f + (patch.GetAdcValue(0) * 72.f);
    note = int(note_v / voltsPerNote);
    freq = mtof(note + coarse_tune);

    /** Set waveform **/
    float mode = 5.0 * patch.GetAdcValue(1);
    if (mode < 1.0) {
      osc.SetWaveform(Oscillator::WAVE_SIN);
    } else if (mode >= 1.0 and mode < 2.0) {
      osc.SetWaveform(Oscillator::WAVE_TRI);
    } else if (mode >= 2.0 and mode < 3.0) {
      osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    } else if (mode >= 3.0 and mode < 4.0) {
      osc.SetWaveform(Oscillator::WAVE_RAMP);
    } else if (mode >= 4.0) {
      osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE);
    }

    /** Set envelope's attack value **/
    float atk_pot = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_3));
    float atk_cv = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_6));
    attack = 0.0001 + (atk_pot + atk_cv) / 5.0;
    adenv.SetTime(ADENV_SEG_ATTACK, attack);

    /** Set envelope's decay value **/
    float dec_pot = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_4));
    float dec_cv = patch.AnalogReadToVolts(analogRead(PIN_PATCH_SM_CV_7));
    decay = 0.0001 + (dec_pot + dec_cv) / 5.0;
    adenv.SetTime(ADENV_SEG_DECAY, decay);

    /** Handle gate inputs **/
    bool btn_state = button.Pressed();
    bool gate_state = patch.gateIns[0].State();
    if (gate_state or btn_state) {
      adenv.Trigger();
      digitalWrite(PIN_PATCH_SM_GATE_OUT_1, HIGH);
      digitalWrite(PIN_PATCH_SM_GATE_OUT_2, HIGH);
    } else {
      digitalWrite(PIN_PATCH_SM_GATE_OUT_1, LOW);  
      digitalWrite(PIN_PATCH_SM_GATE_OUT_2, LOW);  
    }

    /** Toggle whether to use the internal vca. Down for continuous output. **/
    vca_toggle_state = toggle.Pressed();
    
}
