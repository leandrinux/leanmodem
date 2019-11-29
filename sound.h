#ifndef sound_h
#define sound_h

#define guards(c,str,snd) if(!(c)){stream->println(str);sound(snd);return;}

enum SoundCode {
  SND_BEEP = 1,
  SND_GOOD = 2,
  SND_ERROR = 3,
  SND_ALERT = 4,
  SND_CONNECTED = 5
};

void sound_setPin(int pin);
void sound_setMute(bool mute);
void sound(SoundCode code);

#endif
