#ifndef Decay_h
#define Decay_h

#include <Audio.h>

/*
 * Decay
 */
class Decay : public AudioStream {
private:
  audio_block_t *inputQueueArray[2];
  float value = INT16_MIN;
  float coeff = 0.00005;

public:
  Decay();
  void update(void);
  void setCoeff(float coeff);
};

/**
 * Constructor
 */
inline Decay::Decay()
  : AudioStream(2, inputQueueArray) {
}

inline void Decay::update() {
  audio_block_t *triggerBlock = receiveReadOnly(0);
  audio_block_t *coeffBlock = receiveReadOnly(1);
  audio_block_t *outputBlock = allocate();

  if (outputBlock) {

    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {

      if (coeffBlock) {
        this->coeff = map((float)coeffBlock->data[i], (float)INT16_MIN, (float)INT16_MAX, (float)0.001, (float)0.00001);
      }

      this->value = (this->coeff * INT16_MIN) + (1.0f - this->coeff) * this->value;

      if (triggerBlock && triggerBlock->data[i] > 0) {
        this->value = INT16_MAX;
      }

      outputBlock->data[i] = (int16_t)this->value;
    }

    transmit(outputBlock, 0);
    release(outputBlock);
    if (coeffBlock) {
      release(coeffBlock);
    }
    if (triggerBlock) {
      release(triggerBlock);
    }
  }
}
inline void Decay::setCoeff(float coeff) {
  if (coeff > 1) {
    coeff = 1;
  }
  if (coeff < 0) {
    coeff = 0;
  }
  this->coeff = coeff;
}

#endif
