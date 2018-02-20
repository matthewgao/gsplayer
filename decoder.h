
#pragma once
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}
#include <stdio.h>
#include "display.h"
#include <iostream>
#include <string>
#include <list>
#include <memory>
#include <stdio.h>
#include <iostream>

using namespace std;

class MediaDecoder{
public:
    // MediaDecoder();
    MediaDecoder(string file);
    ~MediaDecoder();
    AVFrame* GetAFrame();
    void Flush();
    void PushFrame(AVFrame *frame);
    AVFrame* PopFrame();
    void ShowFrame();

    AVFormatContext* GetFormatCtx(){
        return this->m_format_context;
    }

    AVCodecContext* GetVideoCodecCtx(){
        return this->m_codec_v_context;
    }

    int GetHeight(){
        return this->m_video_height;
    }

    int GetWidth(){
        return this->m_video_width;
    }
private:
    shared_ptr<list<AVFrame*> > m_video_frame;
    shared_ptr<list<AVFrame*> > m_audio_frame;
    int m_video_index;
    int m_audio_index;
    int m_video_height;
    int m_video_width;
    AVFormatContext *m_format_context;
    AVCodec *m_v_codec;
    AVCodec *m_a_codec;
    AVCodecContext *m_codec_v_context;
    AVCodecParameters *m_codec_v_params;
};