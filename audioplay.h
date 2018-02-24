#pragma once
extern "C" {
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_audio.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include "libswresample/swresample.h" 
}
#include <stdio.h>
#include <iostream>
#include <thread>
#include "decoder.h"

using namespace std;
#define MAX_AUDIO_FRAME_SIZE 192000

//forward delcare because it's cycle including
class MediaDecoder;

class AudioPlay : public SDLBase {
public:
    AudioPlay(AVCodecContext *codec_a_context, MediaDecoder* decoder);
    struct SwrContext *GetSwrContext(){
        return this->m_au_convert_ctx;
    }

    void InitAudioDevice();
    static void AudioCallback(void*  userdata, Uint8* stream,int len);

    void SetAudioCodecContext(AVCodecContext *codec_a_context){
        this->m_codec_a_context = codec_a_context;
    }
    void SetMediaDecoder(MediaDecoder *decoder){
        this->m_decoder = decoder;
    }

    void Play(){
         SDL_PauseAudio(0);
    }

    AVFrame* PopAudioFrame();
private:
    struct SwrContext *m_au_convert_ctx;
    AVCodecContext *m_codec_a_context;
    MediaDecoder *m_decoder;
};