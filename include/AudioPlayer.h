#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <Arduino.h>

bool audioInit();
bool audioReinit();
String audioGetInfo();
bool playFile(const char *path);
void stopPlayback();
bool isPlaying();

// Volume control (DFPlayer volume range: 0..30)
bool audioSetVolume(int vol);
int audioGetVolume();

#endif // AUDIOPLAYER_H
