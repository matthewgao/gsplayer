#include "sdlbase.h"
#include <stdio.h>
#include "decoder.h"

void SDLBase::ListenEvent(MediaDecoder *decoder){
    SDL_Event e;
    while( !this->m_should_exit )
    {
        if (SDL_PollEvent( &e ) == 0){
            // this_thread::sleep_for(chrono::milliseconds(200));
        }
        decoder->ShowFrame();
        //User requests quit
        if( e.type == SDL_QUIT )
        {
            this->m_should_exit = true;
        }

        if (e.window.event == SDL_WINDOWEVENT_RESIZED){
            int new_width = e.window.data1;
            int new_height = e.window.data2;
            float aspectRatio = (float)new_width/(float)new_height;
            float ASPECT_RATIO = (float)this->width/(float)this->height;
            if(aspectRatio != ASPECT_RATIO) {
                // if(aspectRatio > ASPECT_RATIO) {
                    new_height = (1.f / ASPECT_RATIO) * new_width; 
                // }
                // else {
                //     new_width = ASPECT_RATIO * new_height; 
                // }

                this->width = new_width;
                this->height = new_height;

                printf("Setting window size to %d, %d, aspect ratio: %f\n", 
                    new_width, new_height, (float)new_width/(float)new_height);
            }
        }
    }
}