#pragma once
extern "C" {
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_video.h>
#include <SDL_surface.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}
#include <stdio.h>
#include <iostream>
#include <thread>

using namespace std;

class SDLBase{
public:
    SDLBase():m_should_exit(false){
        int ret = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
        if (ret != 0) {
            SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
            SDL_Quit();
            return;
        }
    }
    void ListenEvent();
protected:
    bool m_should_exit;
};