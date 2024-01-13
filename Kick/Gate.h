#ifndef Gate_h
#define Gate_h

#include <Audio.h>

/*
 * Gate
 */
class Gate : public AudioStream {
private:
  audio_block_t *inputQueueArray[1];
  // void (*gateOpenCallback)(void);
  // void (*gateCloseCallback)(void);
  float avg = 0;

public:
  Gate();
  void update(void);
  // void onGateOpen(void (*)(void));
  // void onGateClose(void (*)(void));
};

/**
 * Constructor
 */
inline Gate::Gate()
  : AudioStream(1, inputQueueArray) {
}

inline void Gate::update(void) {
  // Receive input data
  audio_block_t *inputBlock = receiveReadOnly(0);
  audio_block_t *outputBlock = allocate();

  if (outputBlock) {
    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      if (inputBlock) {
        this->avg = (0.05 * inputBlock->data[i]) + (1.0f - 0.05) * this->avg;
      }

      outputBlock->data[i] = this->avg >= 0 ? INT16_MAX : INT16_MIN;
    }

    // if (this->avg >= 0) {
    //   if (this->gateOpenCallback) {
    //     // Serial.println("ON");
    //     this->gateOpenCallback();
    //   }
    // } else {
    //   if (this->gateCloseCallback) {
    //     // Serial.println("OFF");
    //     this->gateCloseCallback();
    //   }
    // }

    transmit(outputBlock);
    release(outputBlock);
  }

  if (inputBlock) {
    release(inputBlock);
  }
}

// inline void Gate::onGateOpen(void (*callback)(void)) {
//   this->gateOpenCallback = callback;
// }

// inline void Gate::onGateClose(void (*callback)(void)) {
//   this->gateCloseCallback = callback;
// }

#endif
