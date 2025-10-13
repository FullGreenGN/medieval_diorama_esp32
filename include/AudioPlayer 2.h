#pragma once
#include <Arduino.h>
#include <FS.h>

// By default, do not include the external <Audio.h> library because some forks expect
// unavailable headers (e.g. NetworkClient.h) on this board. To enable the real library,
// define USE_AUDIO_LIBRARY in your build (platformio.ini) and make sure the library is compatible.

#ifdef USE_AUDIO_LIBRARY
  #include <Audio.h>
  #define AUDIO_LIB_AVAILABLE 1
#else
  #define AUDIO_LIB_AVAILABLE 0
#endif

#if !AUDIO_LIB_AVAILABLE
// Minimal stub of the Audio class used by the project so compilation succeeds
class Audio {
public:
    void setPinout(int bclk, int lrc, int dout) {}
    void setVolume(int vol) {}
    // Accept either fs::FS or SD-like first arg; use template-like overload via void*
    void connecttoFS(void* /*fs*/, const char* /*path*/) {}
    void loop() {}
    void stopSong() {}
};
#endif

class AudioPlayer {
public:
    AudioPlayer(int bclk, int lrc, int dout);
    void begin();
    void playMP3(const char *path);
    void playWav(const char *path);
    void loop(); // Must be called in loop()
    void stop();

private:
    Audio audio;
    int _bclk, _lrc, _dout;
    bool _isPlaying = false;
};
