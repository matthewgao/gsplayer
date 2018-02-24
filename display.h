#pragma once
extern "C" {
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_video.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}
#include "sdlbase.h"

using namespace std;

class Display : public SDLBase{
public:
    Display();
    Display(int width, int height);
    ~Display();
    void ShowFrame();
    bool SetTexture(AVFrame *frame);

private:
    int SDLInit(SDL_Window **window, SDL_Renderer **renderer);
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int height;
    int width;
};