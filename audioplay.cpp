#include "audioplay.h"

AudioPlay::AudioPlay(AVCodecContext *codec_a_context, MediaDecoder* decoder):
    m_codec_a_context(codec_a_context),
    m_decoder(decoder)
{
    InitAudioDevice();
}

void AudioPlay::AudioCallback(void* userdata, Uint8* stream,int len){
    SDL_memset(stream, 0, len);
    AudioPlay* md = (AudioPlay*)userdata;
    AVFrame *f = md->PopAudioFrame();
    if(f == nullptr){
        return;
    }

    uint8_t* out_a_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
    int buffer_size = av_samples_get_buffer_size(NULL, 2 ,f->nb_samples,AV_SAMPLE_FMT_S16, 1);
    swr_convert(md->GetSwrContext(), &(out_a_buffer), MAX_AUDIO_FRAME_SIZE, (const uint8_t **)f->data, f->nb_samples);
    // cout<<"callback len "<<len<<endl;
    // cout<<"callback lnb_samplesen "<<f->nb_samples<<endl;
    // cout<<"buffer size "<<buffer_size<<endl;
    SDL_MixAudio(stream, out_a_buffer, buffer_size, 100);
    // SDL_MixAudio(stream, f->data[1], f->linesize[1], 100);
    av_free(out_a_buffer);
    av_frame_unref(f);
    av_frame_free(&f);
}

void AudioPlay::InitAudioDevice(){
    SDL_AudioSpec want;
    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */

    want.freq = this->m_codec_a_context->sample_rate;
    want.format = AUDIO_S16SYS;
    want.channels = this->m_codec_a_context->channels;
    want.samples = this->m_codec_a_context->frame_size;
    want.silence = 0;
    want.callback = AudioPlay::AudioCallback; /* you wrote this function elsewhere. */
    want.userdata = this;

    cout<<"sample_rate "<<this->m_codec_a_context->sample_rate<<endl;
    cout<<"channels "<<this->m_codec_a_context->channels<<endl;
    cout<<"samples "<<this->m_codec_a_context->frame_size<<endl;
    cout<<"sample_fmt "<<this->m_codec_a_context->sample_fmt<<endl;

    if (SDL_OpenAudio(&want, nullptr) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        exit(127);
    }

    this->m_au_convert_ctx = swr_alloc();  
    this->m_au_convert_ctx = swr_alloc_set_opts(this->m_au_convert_ctx, 
        AV_CH_LAYOUT_STEREO, 
        AV_SAMPLE_FMT_S16,
        this->m_codec_a_context->sample_rate,  
        av_get_default_channel_layout(this->m_codec_a_context->channels),
        this->m_codec_a_context->sample_fmt, 
        this->m_codec_a_context->sample_rate, 0, nullptr);  
    swr_init(this->m_au_convert_ctx);  
}

AVFrame* AudioPlay::PopAudioFrame(){
    return this->m_decoder->PopAudioFrame();
}