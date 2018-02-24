#include "sdlbase.h"

void SDLBase::ListenEvent(){
    SDL_Event e;
    while( !this->m_should_exit )
    {
        if (SDL_PollEvent( &e ) == 0){
            this_thread::sleep_for(chrono::milliseconds(200));
        }
        //User requests quit
        if( e.type == SDL_QUIT )
        {
            this->m_should_exit = true;
        }
    }
}