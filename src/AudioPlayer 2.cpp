#include "AudioPlayer.h"
#include "SD.h"

AudioPlayer::AudioPlayer(int bclk, int lrc, int dout)
    : _bclk(bclk), _lrc(lrc), _dout(dout) {}

void AudioPlayer::begin() {
    audio.setPinout(_bclk, _lrc, _dout);
    audio.setVolume(15); // 0-21
    Serial.println("Audio output initialized.");
}

void AudioPlayer::playMP3(const char *path) {
    if (!SD.exists(path)) {
        Serial.println("MP3 file not found!");
        return;
    }

    audio.connecttoFS(SD, path);
    _isPlaying = true;
    Serial.printf("Playing MP3: %s\n", path);
}

void AudioPlayer::playWav(const char *path) {
    if (!SD.exists(path)) {
        Serial.println("WAV file not found!");
        return;
    }

    audio.connecttoFS(SD, path);
    _isPlaying = true;
    Serial.printf("Playing WAV: %s\n", path);
}

void AudioPlayer::loop() {
    if (_isPlaying) {
        audio.loop();
    }
}

void AudioPlayer::stop() {
    audio.stopSong();
    _isPlaying = false;
    Serial.println("Playback stopped.");
}
