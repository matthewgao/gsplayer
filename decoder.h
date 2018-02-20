
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
#include <mutex>
#include <condition_variable>
#include <thread>

using namespace std;

class MediaDecoder{
public:
    // MediaDecoder();
    MediaDecoder(string file);
    ~MediaDecoder();
    AVFrame* GetAFrame();
    void Flush();
    void PushVideoFrame(AVFrame *frame);
    AVFrame* PopVideoFrame();
    void ShowFrame();
    void Decoder();

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

    //mutex is not copyable so MediaDecoder must not copyable either
    MediaDecoder (const MediaDecoder&) = delete;
    MediaDecoder& operator= (const MediaDecoder&) = delete;

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
    mutex m_video_list_mutex;
    mutex m_audio_list_mutex;
    condition_variable_any m_video_cv;
    condition_variable_any m_audio_cv;
    shared_ptr<Display> m_display;
    AVRational m_time_base;
    int64_t m_start_time;
};