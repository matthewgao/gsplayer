#include "decoder.h"
#include <chrono>

MediaDecoder::MediaDecoder(string file):m_video_index(-1),m_audio_index(-1){
    this->m_audio_frame = make_shared<list<AVFrame*> >();
    this->m_video_frame = make_shared<list<AVFrame*> >();

    av_register_all();
    avformat_network_init();
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
        if(this->m_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){  
            this->m_video_index = i;
        }else if(this->m_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
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

    // this->m_codec_v_context->thread_count = 8;
    // this->m_codec_v_context->thread_type = FF_THREAD_FRAME;
    AVDictionary *opts = nullptr;
    // av_dict_set_int(&opts, "threads", 8, 0);
    av_dict_set(&opts, "threads", "auto", 0);

    ret = avcodec_open2(this->m_codec_v_context, this->m_v_codec, &opts);
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
    this->m_audioplay = make_shared<AudioPlay>(this->m_codec_a_context, this);

    this->m_time_base = this->m_format_context->streams[this->m_video_index]->time_base;
    this->m_start_time = 0;
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
    
    // cout<<"video queue size"<<this->m_video_frame->size()<<endl;
    while(this->m_video_frame->size() > 20){
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
        std::cerr << "Waiting... \n";
        this->m_video_cv.wait(lk);
        // this->m_video_cv.wait(lk, [&]{return !this->m_video_frame->empty();});
        std::cerr << "...finished waiting. not empty\n";
    }
    // cout<<"pop video"<<endl;
    AVFrame *f = this->m_video_frame->front();
    this->m_video_frame->pop_front();
    return f;
}

void MediaDecoder::ShowFrame(){
    while(!this->m_display->ShouldExit()){
        AVFrame* frame = this->PopVideoFrame();
        //PTS control
        // cout<<"show a frame"<<endl;
        if(this->m_start_time == 0){
            this->m_start_time = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
            this->m_start_pts = frame->pts;
            this->m_display->SetTexture(frame);
            //this->m_display->SetWindowsSize();
            this->m_display->ShowFrame();
            av_frame_unref(frame);
            av_frame_free(&frame);
        }else{
            cout<<"time_base "<<av_q2d(this->m_time_base)<<endl;
            cout<<"start_pts "<<this->m_start_pts<<endl;
            cout<<"pts "<<frame->pts<<endl;
            // cout<<"pts best"<<av_frame_get_best_effort_timestamp(frame)<<endl;
            cout<<"time_base num "<<this->m_time_base.num<<endl;
            cout<<"time_base den "<<this->m_time_base.den<<endl;
            cout<<"pts in sec "<<av_frame_get_best_effort_timestamp(frame)*av_q2d(this->m_time_base)<<endl;
            int64_t pts = av_frame_get_best_effort_timestamp(frame)*av_q2d(this->m_time_base)*1000*1000;
            int64_t now = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
            // cout<<"show "<<pts<<" "<<now<<endl;
            int64_t need_sleep = (this->m_start_time- now) + (int64_t(pts)-this->GetStartPtsInMicroSec()) ;
            this->m_display->SetTexture(frame);
            cout<<"time to wait "<<need_sleep<<endl;
            if (need_sleep > 0){
                this_thread::sleep_for(chrono::microseconds(need_sleep));
            }
            this->m_display->ShowFrame();
            av_frame_unref(frame);
            av_frame_free(&frame);
        }
    }
    cout<<"end of ShowFrame"<<endl;
}

int64_t MediaDecoder::GetStartPtsInMicroSec(){
    return this->m_start_pts*av_q2d(this->m_time_base)*1000*1000;
}

void MediaDecoder::PlaySound(){
    this->m_audioplay->Play();
}

void MediaDecoder::Decoder(){
    AVFrame *pFrame = av_frame_alloc();
    while(!this->m_display->ShouldExit()){
        // cout<<"Decoder loop"<<endl;
        AVPacket *pPacket = av_packet_alloc();
        if (pPacket == nullptr || this->m_format_context == nullptr){
             cout<<"nullptr"<<endl;
             continue;
        } 
        if (av_read_frame(this->m_format_context, pPacket) < 0){
            cout<<"end"<<endl;
            break;
        }
        // cout<<"av_read_frame v"<<endl;
        if (pPacket->stream_index == this->m_video_index) {
            int ret = avcodec_send_packet(this->m_codec_v_context, pPacket);
            // cout<<"avcodec_send_packet v"<<endl;
            if (ret != 0){
                cerr<<"fail to send packet: "<<ret<<endl;
                continue;
            }

            ret = avcodec_receive_frame(this->m_codec_v_context, pFrame);
            // cout<<"avcodec_receive_frame v"<<endl;
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
            // cout<<"avcodec_send_packet a"<<endl;
            if (ret != 0){
                cerr<<"fail to send audio packet: "<<ret<<endl;
                continue;
            }

            ret = avcodec_receive_frame(this->m_codec_a_context, pFrame);
            // cout<<"avcodec_receive_frame a"<<endl;
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
    cout<<"end of decoding"<<endl;
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
