#include "decoder.h"
#include <chrono>

MediaDecoder::MediaDecoder(string file):m_video_index(-1),m_audio_index(-1){
    this->m_audio_frame = make_shared<list<AVFrame*> >();
    this->m_video_frame = make_shared<list<AVFrame*> >();

    av_register_all();
    this->m_format_context = avformat_alloc_context();
    int ret = avformat_open_input(&(this->m_format_context), file.c_str(), nullptr, nullptr);
    if (ret != 0){
        cerr<<"fail to open file: "<<ret<<endl;
        exit(127);
    }


    ret = avformat_find_stream_info(this->m_format_context, nullptr);
    if (ret != 0){
        cerr<<"fail to find stream: "<<ret<<endl;
        exit(127);
    }

    for(int i = 0; i < this->m_format_context->nb_streams; i++){
        if(this->m_format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){  
            this->m_video_index = i;
        }else if(this->m_format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            this->m_audio_index = i;
        }
    }

    this->m_codec_v_params = this->m_format_context->streams[this->m_video_index]->codecpar;
    this->m_v_codec = avcodec_find_decoder(this->m_codec_v_params->codec_id);
    this->m_video_height = this->m_codec_v_params->height;
    this->m_video_width = this->m_codec_v_params->width;

    this->m_codec_v_context = avcodec_alloc_context3(this->m_v_codec);
    ret = avcodec_parameters_to_context(this->m_codec_v_context, this->m_codec_v_params);
    if (ret != 0){
        cerr<<"fail to set this->m_codec_v_params context: "<<ret<<endl;
        exit(127);
    }

    ret = avcodec_open2(this->m_codec_v_context, this->m_v_codec, nullptr);
    if (ret != 0){
        cerr<<"fail to open this->m_v_codec: "<<ret<<endl;
        exit(127);
    }

    this->m_codec_a_params = this->m_format_context->streams[this->m_audio_index]->codecpar;
    this->m_a_codec = avcodec_find_decoder(this->m_codec_a_params->codec_id);

    this->m_codec_a_context = avcodec_alloc_context3(this->m_a_codec);
    ret = avcodec_parameters_to_context(this->m_codec_a_context, this->m_codec_a_params);
    if (ret != 0){
        cerr<<"fail to set this->m_codec_a_params context: "<<ret<<endl;
        exit(127);
    }

    ret = avcodec_open2(this->m_codec_a_context, this->m_a_codec, nullptr);
    if (ret != 0){
        cerr<<"fail to open this->m_a_codec: "<<ret<<endl;
        exit(127);
    }

    cout<<"--------------- File Information ----------------"<<endl;  
    av_dump_format(this->m_format_context, 0, file.c_str(), 0);  
    cout<<"-------------------------------------------------"<<endl;

    this->m_display = make_shared<Display>(this->m_video_width, this->m_video_height);

    this->m_time_base = this->m_format_context->streams[this->m_video_index]->time_base;
    this->m_start_time = 0;

    // this->InitAudioDevice();
    this->m_audioplay = make_shared<AudioPlay>(this->m_codec_a_context, this);
    // this->m_out_a_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
}

MediaDecoder::~MediaDecoder(){
    avformat_close_input(&(this->m_format_context));
    avformat_free_context(this->m_format_context);
    avcodec_free_context(&(this->m_codec_v_context));
    avcodec_free_context(&(this->m_codec_a_context));
}


AVFrame* MediaDecoder::GetAFrame(){
    return nullptr;
}

void MediaDecoder::Flush(){}
void MediaDecoder::PushVideoFrame(AVFrame *frame){
    if(frame == nullptr){
        return;
    }
    
    // cout<<"size"<<this->m_video_frame->size()<<endl;
    while(this->m_video_frame->size() > 10){
        // cout<<"sleep 200ms"<<endl;
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    lock_guard<std::mutex> guard(this->m_video_list_mutex);
    this->m_video_frame->push_back(frame);
    this->m_video_cv.notify_all();
}

AVFrame* MediaDecoder::PopVideoFrame(){
    // lock_guard<std::mutex> guard(this->m_video_list_mutex);
    std::unique_lock<std::mutex> lk(this->m_video_list_mutex);
    if(this->m_video_frame->empty()){
        // std::cerr << "Waiting... \n";
        this->m_video_cv.wait(lk);
        // this->m_video_cv.wait(lk, [&]{return !this->m_video_frame->empty();});
        // std::cerr << "...finished waiting. not empty\n";
    }
    AVFrame *f = this->m_video_frame->front();
    this->m_video_frame->pop_front();
    return f;
}

void MediaDecoder::ShowFrame(){
    while(!this->m_display->ShouldExit()){
        AVFrame* frame = this->PopVideoFrame();
        //PTS control
        if(this->m_start_time == 0){
            this->m_start_time = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            this->m_display->SetTexture(frame);
            this->m_display->ShowFrame();
            av_frame_unref(frame);
            av_frame_free(&frame);
        }else{
            // cout<<"show "<<av_frame_get_best_effort_timestamp(frame)*av_q2d(this->m_time_base)<<endl;
            int64_t pts = av_frame_get_best_effort_timestamp(frame)*av_q2d(this->m_time_base)*1000;
            int64_t now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            // cout<<"show "<<pts<<" "<<now<<endl;
            int64_t need_sleep = this->m_start_time + int64_t(pts) - now;
            this->m_display->SetTexture(frame);
            // cout<<"time to wait "<<need_sleep<<endl;
            if (need_sleep > 0){
                this_thread::sleep_for(chrono::milliseconds(need_sleep));
            }
            this->m_display->ShowFrame();
            av_frame_unref(frame);
            av_frame_free(&frame);
        }
    }
}

void MediaDecoder::PlaySound(){
    this->m_audioplay->Play();
}

void MediaDecoder::Decoder(){
    AVFrame *pFrame = av_frame_alloc();
    while(!this->m_display->ShouldExit()){
        AVPacket *pPacket = av_packet_alloc();
        if (pPacket == nullptr || this->m_format_context == nullptr){
             cout<<"nullptr"<<endl;
             continue;
        } 
        if (av_read_frame(this->m_format_context, pPacket) < 0){
            cout<<"end"<<endl;
            break;
        }

        if (pPacket->stream_index == this->m_video_index) {
            int ret = avcodec_send_packet(this->m_codec_v_context, pPacket);
            if (ret != 0){
                cerr<<"fail to send packet: "<<ret<<endl;
                continue;
            }

            ret = avcodec_receive_frame(this->m_codec_v_context, pFrame);
            if (ret == AVERROR(EAGAIN)){
                cerr<<"try again: "<<ret<<av_err2str(ret)<<endl;
                continue;
            }

            if (ret != 0){
                cerr<<"fail to receive packet: "<<ret<<av_err2str(ret)<<endl;
                break;
            }

            AVFrame* newFrame = av_frame_clone(pFrame);
            this->PushVideoFrame(newFrame);
        }

        if (pPacket->stream_index == this->m_audio_index) {
            int ret = avcodec_send_packet(this->m_codec_a_context, pPacket);
            if (ret != 0){
                cerr<<"fail to send audio packet: "<<ret<<endl;
                continue;
            }

            ret = avcodec_receive_frame(this->m_codec_a_context, pFrame);
            if (ret == AVERROR(EAGAIN)){
                cerr<<"try audio again: "<<ret<<av_err2str(ret)<<endl;
                continue;
            }

            if (ret != 0){
                cerr<<"fail to receive audio packet: "<<ret<<av_err2str(ret)<<endl;
                break;
            }

            AVFrame* newFrame = av_frame_clone(pFrame);
            this->PushAudioFrame(newFrame);
        }

        av_packet_free(&pPacket);
        av_frame_unref(pFrame);
    }
}

void MediaDecoder::Polling(){
    this->m_display->ListenEvent();
}

void MediaDecoder::PushAudioFrame(AVFrame *frame){
    if(frame == nullptr){
        return;
    }
    
    lock_guard<std::mutex> guard(this->m_audio_list_mutex);
    this->m_audio_frame->push_back(frame);
    // this->m_video_cv.notify_all();
}

AVFrame* MediaDecoder::PopAudioFrame(){
    lock_guard<std::mutex> guard(this->m_audio_list_mutex);
    if(this->m_audio_frame->empty()){
        return nullptr;
    }

    // cout<<"size"<<this->m_audio_frame->size()<<endl;
    AVFrame *f = this->m_audio_frame->front();
    this->m_audio_frame->pop_front();
    return f;
}


// void MediaDecoder::AudioCallback(void* userdata, Uint8* stream,int len){
//     SDL_memset(stream, 0, len);
//     MediaDecoder* md = (MediaDecoder*)userdata;
//     AVFrame *f = md->PopAudioFrame();
//     if(f == nullptr){
//         return;
//     }

//     uint8_t* out_a_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
//     int buffer_size = av_samples_get_buffer_size(NULL, 2 ,f->nb_samples,AV_SAMPLE_FMT_S16, 1);
//     swr_convert(md->GetSwrContext(), &(out_a_buffer), MAX_AUDIO_FRAME_SIZE, (const uint8_t **)f->data, f->nb_samples);
//     cout<<"callback len "<<len<<endl;
//     cout<<"callback lnb_samplesen "<<f->nb_samples<<endl;
//     cout<<"buffer size "<<buffer_size<<endl;
//     SDL_MixAudio(stream, out_a_buffer, buffer_size, 100);
//     // SDL_MixAudio(stream, f->data[1], f->linesize[1], 100);
//     av_free(out_a_buffer);
//     av_frame_unref(f);
//     av_frame_free(&f);
// }

// void MediaDecoder::InitAudioDevice(){
//     SDL_AudioSpec want;
//     SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */

//     want.freq = this->m_codec_a_context->sample_rate;
//     want.format = AUDIO_S16SYS;
//     want.channels = this->m_codec_a_context->channels;
//     want.samples = this->m_codec_a_context->frame_size;
//     want.silence = 0;
//     want.callback = MediaDecoder::AudioCallback; /* you wrote this function elsewhere. */
//     want.userdata = this;

//     cout<<"sample_rate "<<this->m_codec_a_context->sample_rate<<endl;
//     cout<<"channels "<<this->m_codec_a_context->channels<<endl;
//     cout<<"samples "<<this->m_codec_a_context->frame_size<<endl;
//     cout<<"samples "<<this->m_codec_a_context->sample_fmt<<endl;

//     if (SDL_OpenAudio(&want, nullptr) < 0) {
//         SDL_Log("Failed to open audio: %s", SDL_GetError());
//         exit(127);
//     }

//     this->m_au_convert_ctx = swr_alloc();  
//     this->m_au_convert_ctx = swr_alloc_set_opts(this->m_au_convert_ctx, 
//         AV_CH_LAYOUT_STEREO, 
//         AV_SAMPLE_FMT_S16,
//         this->m_codec_a_context->sample_rate,  
//         av_get_default_channel_layout(this->m_codec_a_context->channels),
//         this->m_codec_a_context->sample_fmt, 
//         this->m_codec_a_context->sample_rate, 0, nullptr);  
//     swr_init(this->m_au_convert_ctx);  
// }