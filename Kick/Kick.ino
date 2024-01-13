/*
GHOST IN TRANSLATION
KICK
v1.0.0

Support my work:
https://patreon.com/ghostintranslation
https://www.buymeacoffee.com/ghostintranslation

Music and Merch:
https://ghostintranslation.bandcamp.com/

Social:
https://www.instagram.com/ghostintranslation/
https://www.youtube.com/c/ghostintranslation

Code:
https://github.com/ghostintranslation

My website:
https://www.ghostintranslation.com/
*/

#include <Audio.h>
#include "Kick.h"

// Instanciation of Kick
Kick *kick = Kick::getInstance();

AudioOutputI2S i2s;
AudioControlSGTL5000 audioBoard;

void setup()
{
  Serial.begin(115200);

  while (!Serial && millis() < 2500)
    ; // wait for serial monitor

  // Audio connections require memory to work.
  AudioMemory(100);

  audioBoard.enable();
  audioBoard.volume(0.2);

  kick->init();
  new AudioConnection(*kick->getOutput(), 0, i2s, 0);
  new AudioConnection(*kick->getOutput(), 0, i2s, 1);
}

void loop()
{
}
