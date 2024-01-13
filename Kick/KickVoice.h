#ifndef Voice_h
#define Voice_h

#include <Audio.h>
#include "Motherboard/Vca.h"
#include "Decay.h"
// #include "Gate.h"
#include "Trigger.h"


// For some reason having the waveshaper object in the class crashes the Teensy...
AudioEffectWaveshaper waveshaper;

/*
 * Voice
 */
class KickVoice : public AudioStream {
private:
  audio_block_t *inputQueueArray[7];

  // Gate *gate;
  Trigger *trigger;
  Decay *decay1;
  Decay *decay2;
  Decay *noiseDecay;
  Vca *vca1;
  Vca *vca2;
  Vca *vca3;
  Vca *noiseVca;
  AudioSynthWaveformModulated *sine1;
  AudioSynthWaveformModulated *sine2;
  AudioEffectMultiply *multiply;
  AudioSynthNoiseWhite *noise;
  AudioFilterStateVariable *noiseFilter;
  AudioFilterStateVariable *noiseFilter2;
  AudioMixer4 *mixer;
  AudioMixer4 *mixer2;
  AudioAmplifier *amp;

  AudioRecordQueue *audioOutQueue;
  AudioPlayQueue *triggerQueue;
  AudioPlayQueue *minQueue;
  AudioPlayQueue *maxQueue;
  AudioPlayQueue *decayQueue;
  AudioPlayQueue *ampDecayQueue;
  AudioPlayQueue *clickQueue;

  int16_t triggerData[AUDIO_BLOCK_SAMPLES];
  int16_t minData[AUDIO_BLOCK_SAMPLES];
  int16_t maxData[AUDIO_BLOCK_SAMPLES];
  int16_t decayData[AUDIO_BLOCK_SAMPLES];
  int16_t ampDecayData[AUDIO_BLOCK_SAMPLES];
  int16_t clickData[AUDIO_BLOCK_SAMPLES];
  int distAmount = 0;

public:
  KickVoice();
  void update(void);
};

/**
 * Constructor
 */
inline KickVoice::KickVoice()
  : AudioStream(7, inputQueueArray) {

  this->trigger = new Trigger();
  this->decay1 = new Decay();
  this->decay2 = new Decay();
  this->noiseDecay = new Decay();
  this->noiseDecay->setCoeff(0.05);
  this->vca1 = new Vca();
  this->vca2 = new Vca();
  this->vca3 = new Vca();
  this->noiseVca = new Vca();

  this->sine1 = new AudioSynthWaveformModulated();
  this->sine1->begin(WAVEFORM_SINE);
  this->sine1->frequencyModulation(5);
  this->sine1->frequency(10);
  this->sine1->amplitude(1);

  this->sine2 = new AudioSynthWaveformModulated();
  this->sine2->begin(WAVEFORM_SINE);
  this->sine2->frequencyModulation(8);
  this->sine2->frequency(2);
  this->sine2->amplitude(1);

  this->noise = new AudioSynthNoiseWhite();
  this->noise->amplitude(0.5);

  this->noiseFilter = new AudioFilterStateVariable();
  this->noiseFilter->frequency(5000);
  this->noiseFilter->octaveControl(5);

  this->mixer = new AudioMixer4();
  this->mixer->gain(0, 0.5);
  this->mixer->gain(1, 0.5);
  this->mixer->gain(2, 0.1);
  this->mixer->gain(3, 0);

  this->mixer2 = new AudioMixer4();
  this->mixer2->gain(0, 0.5);
  this->mixer2->gain(1, 0.75);

  this->audioOutQueue = new AudioRecordQueue();
  this->audioOutQueue->begin();

  this->triggerQueue = new AudioPlayQueue();
  this->maxQueue = new AudioPlayQueue();
  this->minQueue = new AudioPlayQueue();
  this->decayQueue = new AudioPlayQueue();
  this->ampDecayQueue = new AudioPlayQueue();
  this->clickQueue = new AudioPlayQueue();

  new AudioConnection(*this->triggerQueue, 0, *this->trigger, 0);
  new AudioConnection(*this->trigger, 0, *this->decay1, 0);
  new AudioConnection(*this->trigger, 0, *this->decay2, 0);
  new AudioConnection(*this->trigger, 0, *this->noiseDecay, 0);
  new AudioConnection(*this->ampDecayQueue, 0, *this->decay1, 1);
  new AudioConnection(*this->decayQueue, 0, *this->decay2, 1);
  new AudioConnection(*this->minQueue, 0, *this->sine1, 0);
  new AudioConnection(*this->decay2, 0, *this->vca3, 0);
  new AudioConnection(*this->maxQueue, 0, *this->vca3, 1);
  new AudioConnection(*this->vca3, 0, *this->sine2, 0);
  new AudioConnection(*this->sine1, 0, *this->vca1, 0);
  new AudioConnection(*this->decay1, 0, *this->vca1, 1);
  new AudioConnection(*this->sine2, 0, *this->vca2, 0);
  new AudioConnection(*this->decay2, 0, *this->vca2, 1);
  new AudioConnection(*this->noise, 0, *this->noiseFilter, 0);
  new AudioConnection(*this->clickQueue, 0, *this->noiseFilter, 1);
  new AudioConnection(*this->noiseFilter, 0, *this->noiseVca, 0);
  new AudioConnection(*this->noiseDecay, 0, *this->noiseVca, 1);
  new AudioConnection(*this->vca1, 0, *this->mixer, 0);
  new AudioConnection(*this->vca2, 0, *this->mixer, 1);
  new AudioConnection(*this->noiseVca, 0, *this->mixer, 2);
  new AudioConnection(*this->mixer, 0, waveshaper, 0);
  new AudioConnection(*this->mixer, 0, *this->mixer2, 0);
  new AudioConnection(waveshaper, 0, *this->mixer2, 1);
  new AudioConnection(*this->mixer2, 0, *this->audioOutQueue, 0);
}

inline void KickVoice::update(void) {
  // Receive input data
  audio_block_t *maxBlock;
  audio_block_t *minBlock;
  audio_block_t *decayBlock;
  audio_block_t *ampDecayBlock;
  audio_block_t *clickBlock;
  audio_block_t *distBlock;
  audio_block_t *triggerBlock;

  // Max block
  maxBlock = receiveReadOnly(0);

  if (maxBlock) {
    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      this->maxData[i] = maxBlock->data[i];  //(gainBlock->data[i] + 32768) / 2; // Negative values reverse the signal, we don't want that.
    }

    release(maxBlock);
  }

  this->maxQueue->play(this->maxData, AUDIO_BLOCK_SAMPLES);

  // Min block
  minBlock = receiveReadOnly(1);

  if (minBlock) {
    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      this->minData[i] = minBlock->data[i];  //(gainBlock->data[i] + 32768) / 2; // Negative values reverse the signal, we don't want that.
    }

    release(minBlock);
  }

  this->minQueue->play(this->minData, AUDIO_BLOCK_SAMPLES);

  // Decay block
  decayBlock = receiveReadOnly(2);

  if (decayBlock) {
    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      this->decayData[i] = decayBlock->data[i];  //(gainBlock->data[i] + 32768) / 2; // Negative values reverse the signal, we don't want that.
    }

    release(decayBlock);
  }

  this->decayQueue->play(this->decayData, AUDIO_BLOCK_SAMPLES);

  // Amp Decay block
  ampDecayBlock = receiveReadOnly(3);

  if (ampDecayBlock) {
    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      this->ampDecayData[i] = ampDecayBlock->data[i];  //(gainBlock->data[i] + 32768) / 2; // Negative values reverse the signal, we don't want that.
    }

    release(ampDecayBlock);
  }

  this->ampDecayQueue->play(this->ampDecayData, AUDIO_BLOCK_SAMPLES);

  // Click block
  clickBlock = receiveReadOnly(4);

  if (clickBlock) {
    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      this->clickData[i] = clickBlock->data[i];  //(gainBlock->data[i] + 32768) / 2; // Negative values reverse the signal, we don't want that.
    }

    release(clickBlock);
  }

  this->clickQueue->play(this->clickData, AUDIO_BLOCK_SAMPLES);

  // Dist block
  distBlock = receiveReadOnly(5);

  if (distBlock) {
    // float val = ((float)(distBlock->data[0] + 32767.0) / 65536.0);
    // float val = map(((float)(distBlock->data[0] + 32767.0) / 65536.0), 0.0,1.0, -1.0, -0.5);
    // float WAVESHAPE_EXAMPLE[] = {val, 0, 1.0};

    // uint8_t newValue = map(distBlock->data[0] >> 8, INT8_MIN, INT8_MAX, 0, 100);
    int newValue = map((distBlock->data[0] >> 8), INT8_MIN, INT8_MAX, 0, 20);
    if (newValue != this->distAmount) {
      this->distAmount = newValue;

      uint16_t WAVESHAPE_SIZE = 5;
      float WAVESHAPE_EXAMPLE[WAVESHAPE_SIZE];
      float a = (float)this->distAmount / 100;
      // Formula inspired by https://kenny-peng.com/2020/11/23/teensy_harmonic_distortion.html
      for (uint16_t i = 0; i < WAVESHAPE_SIZE; i++) {
        float x = (float)i * 1.0 / (float)WAVESHAPE_SIZE - 1.0;
        WAVESHAPE_EXAMPLE[i] = ((sin(x) + a * sin(2 * x - PI / 2) + a) / 1.0 + 2.0 * a) - 0.5;

        // WAVESHAPE_EXAMPLE[i] =
        //   (3.0 + this->distAmount) * x * 20.0 * deg / (PI + this->distAmount * abs(x));
      }
      // shape(waveshapeData, WAVESHAPE_SIZE);

      // normalize the waveshaper to -1dB Vpk
      float max_abs_val = 0;
      for (int i = 0; i < WAVESHAPE_SIZE; i++) {
        float abs_val = abs(WAVESHAPE_EXAMPLE[i]);
        if (abs_val > max_abs_val) max_abs_val = abs_val;
      }
      for (int i = 0; i < WAVESHAPE_SIZE; i++) WAVESHAPE_EXAMPLE[i] *= pow(10.0, -1.0 / 20.0) / max_abs_val;

      waveshaper.shape(WAVESHAPE_EXAMPLE, sizeof(WAVESHAPE_EXAMPLE) / sizeof(WAVESHAPE_EXAMPLE[0]));
    }

    release(distBlock);
  }

  // this->distQueue->play(this->distData, AUDIO_BLOCK_SAMPLES);

  // Trigger block
  // Receiving data from input of the block, and passing it to the queue which is connected to the other objects
  triggerBlock = receiveReadOnly(6);

  if (triggerBlock) {
    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      this->triggerData[i] = triggerBlock->data[i];  //(gainBlock->data[i] + 32768) / 2; // Negative values reverse the signal, we don't want that.
    }

    release(triggerBlock);
  }

  this->triggerQueue->play(this->triggerData, AUDIO_BLOCK_SAMPLES);

  // Transmitting the audio queue
  if (this->audioOutQueue->available()) {
    while (this->audioOutQueue->available()) {
      audio_block_t *audioBlock = allocate();
      if (audioBlock) {
        int16_t *queueData = this->audioOutQueue->readBuffer();
        for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
          audioBlock->data[i] = queueData[i];
        }
        transmit(audioBlock, 0);
        release(audioBlock);
        this->audioOutQueue->freeBuffer();
      }
    }
  }


  // for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
  //   // Serial.printf("%d,%d,%d,%d,%d,%d", this->maxData[i], this->minData[i], this->decayData[i], this->ampDecayData[i], this->clickData[i], this->triggerData[i]);
  //   Serial.printf("%d", this->clickData[i]);
  //   Serial.println("");
  // }
}
#endif
