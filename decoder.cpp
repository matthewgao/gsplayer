#include "decoder.h"

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

    cout<<"--------------- File Information ----------------"<<endl;  
    av_dump_format(this->m_format_context, 0, file.c_str(), 0);  
    cout<<"-------------------------------------------------"<<endl;

}
// MediaDecoder::MediaDecoder(string file){
//     MediaDecoder();
// }
MediaDecoder::~MediaDecoder(){
    avformat_close_input(&(this->m_format_context));
    avformat_free_context(this->m_format_context);
    avcodec_free_context(&(this->m_codec_v_context));
}


AVFrame* MediaDecoder::GetAFrame(){
    return nullptr;
}

void MediaDecoder::Flush(){}
void MediaDecoder::PushFrame(AVFrame *frame){
    if(frame == nullptr){
        return;
    }
    // if (frame->stream)
}

AVFrame* MediaDecoder::PopFrame(){
    return nullptr;
}
void MediaDecoder::ShowFrame(){}
