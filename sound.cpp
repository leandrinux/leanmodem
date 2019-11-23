#include "sound.h"
#include <Arduino.h>

int snd_pin = 0;
bool snd_mute = false; 

void sound_setPin(int pin) {
  snd_pin = pin;
}

void sound_setMute(bool mute) {
  snd_mute = mute;
}

void sound(SoundCode code) {
  if (!snd_pin || snd_mute) return;
  switch (code) {
    case SND_BEEP:
      tone(snd_pin, 1000, 100);
      break;
    case SND_GOOD:
      tone(snd_pin, 500, 100);
      break;
    case SND_ERROR:
      tone(snd_pin, 400, 30); delay(40);
      tone(snd_pin, 400, 30); delay(40);
      tone(snd_pin, 400, 30); delay(40);
      break;
    case SND_ALERT:
      for (int i=500;i<1000;i+=10) { tone(snd_pin, i, 3); delay(3); }
      for (int i=1000;i>500;i-=10) { tone(snd_pin, i, 3); delay(3); }
      break;
    case SND_CONNECTED:
      tone(snd_pin, 800, 30); delay(30);
      tone(snd_pin, 1000, 30); delay(30);
      tone(snd_pin, 1200, 30); delay(30);
      break;
  }
}
