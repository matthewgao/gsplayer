extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}
#include <stdio.h>
#include "display.h"
#include "decoder.h"
#include <iostream>
#include <functional>

using namespace std;

void printHexPoint(AVFrame*, int, int);

int main(){
    // shared_ptr<MediaDecoder> decoder = make_shared<MediaDecoder>("SD.mp4");
    shared_ptr<MediaDecoder> decoder = make_shared<MediaDecoder>("sully.mp4");
    std::thread t1(bind(&MediaDecoder::Decoder,decoder));
    std::thread t2(bind(&MediaDecoder::ShowFrame,decoder));
    t1.join();
    t2.detach();
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

