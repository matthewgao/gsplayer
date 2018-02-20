#include "display.h"

using namespace std;

Display::Display(){
    Display(1280, 800);
}

Display::Display(int width, int height):width(width),height(height){
    if(this->SDLInit(&(this->window), &(this->renderer))!=0){
        cerr<<"fail to open windows: "<<endl;
        exit(127);
    }

    this->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (this->texture == NULL) {
        fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
}

void Display::ShowFrame(){
    SDL_SetRenderTarget(this->renderer, this->texture);
    SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
    SDL_RenderPresent(this->renderer);
    return;
}

bool Display::SetTexture(AVFrame *frame){
    SDL_UpdateYUVTexture(
            this->texture, nullptr,  
            frame->data[0], frame->linesize[0],  
            frame->data[1], frame->linesize[1],  
            frame->data[2], frame->linesize[2]
    );
    return true;
}

Display::~Display(){
    SDL_DestroyTexture(this->texture);
}

int Display::SDLInit(SDL_Window **window, SDL_Renderer **renderer){
    int ret = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    if (ret != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if (SDL_CreateWindowAndRenderer(this->width, this->height, SDL_WINDOW_RESIZABLE, window, renderer)!=0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }

    return 0;
}
