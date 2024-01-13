#ifndef Trigger_h
#define Trigger_h

#include <Audio.h>

/*
 * Trigger
 */
class Trigger : public AudioStream {
private:
  audio_block_t *inputQueueArray[1];
  float avg = 0;
  float prevAvg = 0;
  uint8_t triggerActiveCount = 0;

public:
  Trigger();
  void update(void);
};

/**
 * Constructor
 */
inline Trigger::Trigger()
  : AudioStream(1, inputQueueArray) {
}

inline void Trigger::update(void) {
  // Receive input data
  audio_block_t *inputBlock = receiveReadOnly(0);
  audio_block_t *outputBlock = allocate();

  if (outputBlock) {
    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      if (inputBlock) {
        this->avg = (0.05 * inputBlock->data[i]) + (1.0f - 0.05) * this->avg;
      }

      if (this->avg > 0 && this->prevAvg <= 0 && this->triggerActiveCount == 0) {
        this->triggerActiveCount = AUDIO_BLOCK_SAMPLES;
      }

      outputBlock->data[i] = this->triggerActiveCount > 0 ? INT16_MAX : INT16_MIN;

      if (this->triggerActiveCount > 0) {
        this->triggerActiveCount--;
      }

      this->prevAvg = this->avg;
    }

    transmit(outputBlock);
    release(outputBlock);
  }

  if (inputBlock) {
    release(inputBlock);
  }
}

#endif
