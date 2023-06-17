#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <string>
#include <Tiny_File_Dialogs/tinyfiledialogs.h>

const int WIDTH = 1920, HEIGHT = 1080;

bool isPointInRect(int x, int y, const SDL_Rect& rect)
{
    return (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h);
}

std::string formatTime(int seconds)
{
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;
    return std::to_string(minutes) + ":" + (remainingSeconds < 10 ? "0" : "") + std::to_string(remainingSeconds);
}

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        std::cout << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Event windowEvent;

    // Initialize SDL2_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cout << "SDL2_mixer could not initialize: " << Mix_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0)
    {
        std::cout << "SDL_ttf could not initialize: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    // Load the music file
    Mix_Music* music = nullptr;

    // Load the font
    TTF_Font* font = TTF_OpenFont("font.ttf", 24);
    if (font == nullptr)
    {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    bool isMusicPlaying = false;
    int startTime = 0;
    while (!quit)
    {
        while (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
            else if (windowEvent.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX = windowEvent.button.x;
                int mouseY = windowEvent.button.y;

                // Check if the mouse click is inside the button area
                SDL_Rect buttonRect = {(WIDTH - 200) / 2, HEIGHT - 150, 200, 50};
                if (isPointInRect(mouseX, mouseY, buttonRect))
                {
                    // Open file dialog to choose a music file
                    const char* filepath = tinyfd_openFileDialog("Choose Music File", "", 0, nullptr, nullptr, 0);

                    if (filepath != nullptr)
                    {
                        if (music != nullptr)
                        {
                            Mix_FreeMusic(music);
                            music = nullptr;
                        }

                        music = Mix_LoadMUS(filepath);
                        if (music == nullptr)
                        {
                            std::cout << "Failed to load music: " << Mix_GetError() << std::endl;
                        }
                        else
                        {
                            if (Mix_PlayMusic(music, -1) == -1)
                            {
                                std::cout << "Failed to play music: " << Mix_GetError() << std::endl;
                            }
                            else
                            {
                                isMusicPlaying = true;
                                startTime = SDL_GetTicks() / 1000;
                            }
                        }

                        SDL_free((void*)filepath);
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the button
        SDL_Rect buttonRect = {(WIDTH - 200) / 2, HEIGHT - 150, 200, 50};
        SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255); // Purple color
        SDL_RenderFillRect(renderer, &buttonRect);

        // Render the text
        SDL_Color textColor = { 255, 255, 255, 255 }; // White color
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Choose file", textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        int textX = buttonRect.x + (buttonRect.w - textWidth) / 2;  // Center horizontally
        int textY = buttonRect.y + (buttonRect.h - textHeight) / 2; // Center vertically
        SDL_Rect textRect = { textX, textY, textWidth, textHeight };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        if (isMusicPlaying && music != nullptr)
        {
            int currentTime = SDL_GetTicks() / 1000;
            int elapsedSeconds = currentTime - startTime;
            std::string timeText = formatTime(elapsedSeconds);

            SDL_Surface* timeSurface = TTF_RenderText_Solid(font, timeText.c_str(), textColor);
            SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
            int timeWidth = timeSurface->w;
            int timeHeight = timeSurface->h;
            int timeX = (WIDTH - timeWidth) / 2;    // Center horizontally
            int timeY = (HEIGHT - timeHeight) / 2;  // Center vertically
            SDL_Rect timeRect = { timeX, timeY, timeWidth, timeHeight };
            SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);
            SDL_FreeSurface(timeSurface);
            SDL_DestroyTexture(timeTexture);
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    if (music != nullptr)
    {
        Mix_FreeMusic(music);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
