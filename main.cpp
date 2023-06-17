#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

const int WIDTH = 800, HEIGHT = 600;

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);

    if (NULL == window)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Event windowEvent;

    // Initialize SDL audio
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cout << "SDL audio initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL2_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cout << "SDL2_mixer could not initialize: " << Mix_GetError() << std::endl;
        return 1;
    }

    
    // Load the music file
    Mix_Music* music = Mix_LoadMUS("music.mp3");
    if (music == nullptr)
    {
        std::cout << "Failed to load music: " << Mix_GetError() << std::endl;
        return 1;
    }

    // Play the music file
    if (Mix_PlayMusic(music, -1) == -1)
    {
        std::cout << "Failed to play music: " << Mix_GetError() << std::endl;
        return 1;
    }

    while (true)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (SDL_QUIT == windowEvent.type)
            {
                break;
            }
        }
    }



    while (true)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (SDL_QUIT == windowEvent.type)
            {
                break;
            }
        }
    }

    // Cleanup
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    SDL_Quit();

    return EXIT_SUCCESS;
}
