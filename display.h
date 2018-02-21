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

class Display {
public:
    Display();
    Display(int width, int height);
    ~Display();
    void ShowFrame();
    bool SetTexture(AVFrame *frame);
    bool ShouldExit(){return this->m_should_exit;}
    void ListenEvent();

private:
    int SDLInit(SDL_Window **window, SDL_Renderer **renderer);
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int height;
    int width;
    bool m_should_exit;
};