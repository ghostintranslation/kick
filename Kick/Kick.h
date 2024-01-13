#ifndef Kick_h
#define Kick_h

#include <Audio.h>
#include "Motherboard/Input.h"
#include "Motherboard/OutputLed.h"
#include "Trigger.h"
// #include "Gate.h"
#include "Decay.h"
#include "KickVoice.h"

// AudioInputUSB usbIn;
// AudioOutputUSB usbOut;

/*
   Kick
*/
class Kick {

private:
  // Singleton
  static Kick *instance;
  Kick();

  KickVoice *kickVoice;

  // Inputs
  Input *max;
  Input *min;
  Input *decay;
  Input *ampDecay;
  Input *click;
  Input *distortion;
  Input *triggerInput;
  Trigger *trigger;
  // Gate *gate;
  Decay *ledDecay;

  // Leds
  OutputLed *led;

  byte updateMillis = 10;
  elapsedMillis clockUpdate;

  AudioMixer4 *output;

public:
  static Kick *getInstance();
  void init();
  // static void GateOpen();
  // static void GateClose();

  // Audio output
  AudioMixer4 *getOutput();
};

// Singleton pre init
Kick *Kick::instance = nullptr;

/**
   Constructor
*/
inline Kick::Kick() {
}

/**
   Singleton instance
*/
inline Kick *Kick::getInstance() {
  if (!instance)
    instance = new Kick;
  return instance;
}

/**
   Init
*/
inline void Kick::init() {
  this->kickVoice = new KickVoice();

  this->max = new Input(0);
  this->min = new Input(1);
  this->decay = new Input(2);
  this->ampDecay = new Input(3);
  this->click = new Input(4);
  this->distortion = new Input(5);
  this->distortion->setLowPassCoeff(0.0001);
  this->triggerInput = new Input(6);
  this->trigger = new Trigger();
  // this->gate = new Gate();
  // this->gate->onGateOpen(GateOpen);
  // this->gate->onGateClose(GateClose);
  this->ledDecay = new Decay();
  this->ledDecay->setCoeff(0.0005);

  this->led = new OutputLed(0);
  this->led->setStatus(OutputLed::Status::On);
  this->led->setSmoothing(1);

  this->output = new AudioMixer4();
  this->output->gain(0, 1);


  new AudioConnection(*this->max, 0, *this->kickVoice, 0);
  new AudioConnection(*this->min, 0, *this->kickVoice, 1);
  new AudioConnection(*this->decay, 0, *this->kickVoice, 2);
  new AudioConnection(*this->ampDecay, 0, *this->kickVoice, 3);
  new AudioConnection(*this->click, 0, *this->kickVoice, 4);
  new AudioConnection(*this->distortion, 0, *this->kickVoice, 5);
  new AudioConnection(*this->triggerInput, 0, *this->kickVoice, 6);
  new AudioConnection(*this->triggerInput, 0, *this->trigger, 0);
  new AudioConnection(*this->trigger, 0, *this->ledDecay, 0);
  new AudioConnection(*this->ledDecay, 0, *this->led, 0);
  new AudioConnection(*this->kickVoice, 0, *this->output, 0);

  // new AudioConnection(*this->click, 0, usbOut, 0);

  // AudioSynthWaveformDc *dc = new AudioSynthWaveformDc();
  // dc->amplitude(1);
  // new AudioConnection(*dc, 0, *this->led1, 0);
  // new AudioConnection(*this->triggerInput, 0, *this->gate, 0);
  // new AudioConnection(*this->gate, 0, *this->decayEnvelope, 0);
  // new AudioConnection(*this->decay, 0, *this->decayEnvelope, 1);
  // new AudioConnection(*this->decayEnvelope, 0, *this->led1, 0);
}

/**
   Return the audio left output
*/
inline AudioMixer4 *Kick::getOutput() {
  return this->output;
}

// inline void Kick::GateOpen() {
//   // Serial.println("GateOpen");

//   // getInstance()->led1->setStatus(Led::Status::On);
// }

// inline void Kick::GateClose() {
//   // Serial.println("GateClose");
//   // getInstance()->led1->setStatus(Led::Status::Off);
// }

#endif
