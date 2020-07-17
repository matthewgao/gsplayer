extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}
#include <stdio.h>
#include <iostream>
#include <functional>
#include "display.h"
#include "decoder.h"

using namespace std;

void printHexPoint(AVFrame*, int, int);

int main(int argc, char *argv[]){
    if (argc < 2){
        cout<<"missing file"<<endl;
        exit(-1);
    }
    string file(argv[1]);

    shared_ptr<MediaDecoder> decoder = make_shared<MediaDecoder>(file);
    decoder->PlaySound();
    std::thread t1(bind(&MediaDecoder::Decoder, decoder));
    // std::thread t2(bind(&MediaDecoder::Polling, decoder));
    
    t1.detach();
    // t2.detach();

    // decoder->ShowFrame();
    // decoder->PlaySound();
    decoder->Polling();
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

