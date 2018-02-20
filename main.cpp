extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}
#include <stdio.h>
#include "display.h"
#include "decoder.h"
#include <iostream>

using namespace std;

void printHexPoint(AVFrame*, int, int);

int main(){
    MediaDecoder* decoder = new MediaDecoder("in.mp4");
    AVFormatContext *formatContext = decoder->GetFormatCtx();
    AVCodecContext *codecContext = decoder->GetVideoCodecCtx();

    // AVPacket *pPacket = av_packet_alloc();
    AVFrame *pFrame = av_frame_alloc();
    Display *dis = new Display(decoder->GetWidth(),decoder->GetHeight()); 
    for(;;){
        AVPacket *pPacket = av_packet_alloc();
        // cout<<"loop"<<endl;
        if (av_read_frame(formatContext, pPacket) < 0){
            cout<<"break"<<endl;
            break;
        }

        
        if (pPacket->stream_index == 1) {
            continue;
        }
        //  cout<<"index:"<<pPacket->stream_index<<endl;
        // cout<<"loop1"<<endl;
        int ret = avcodec_send_packet(codecContext, pPacket);
        if (ret != 0){
            cerr<<"fail to get packet: "<<ret<<endl;
            continue;
        }

        // cout<<"loop2"<<endl;
        ret = avcodec_receive_frame(codecContext, pFrame);
        // cout<<"loop3"<<endl;
        if (ret == AVERROR(EAGAIN)){
            for (;;){
                ret = avcodec_receive_frame(codecContext, pFrame);
                cout<<ret<<av_err2str(ret)<<endl;
                if (ret == 0 
                // || ret != AVERROR(EAGAIN)
                ){
                    break;
                }
            }
        }
        if (ret != 0){
            cerr<<"fail to receive packet: "<<ret<<av_err2str(ret)<<endl;
            continue;
        }

        
        // cout<<"key_frame:"<<pFrame->key_frame<<endl;

        dis->SetTexture(pFrame);
        dis->ShowFrame();               

        // cout<<"data:"<<pFrame->data[0]<<endl;
        // printHexPoint(pFrame, 2, 10);
        av_packet_unref(pPacket);
        // SDL_FreeSurface(surf);
        // SDL_Delay(formatContext->fps); 
        
    }
    av_frame_free(&pFrame);

    avcodec_free_context(&codecContext);
    avformat_free_context(formatContext);
    return 0;
}

void printHexPoint(AVFrame *frame, int idx, int line_num){
    for (int i =0; i < line_num; i++){
        for (int j=0; j < frame->linesize[idx]; j++){
            printf("%x ", frame->data[idx][j+i*frame->linesize[idx]]);
        }
        printf("-------------\n");
    }
}

