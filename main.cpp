#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <string>
#include <Tiny_File_Dialogs/tinyfiledialogs.h>

const int WIDTH = 1920, HEIGHT = 1080;

bool isPointInRect(int x, int y, const SDL_Rect &rect)
{
    return (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h);
}

std::string formatTime(int seconds)
{
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;
    return std::to_string(minutes) + ":" + (remainingSeconds < 10 ? "0" : "") + std::to_string(remainingSeconds);
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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
    Mix_Music *music = nullptr;
    int musicDuration = 0; // Variable to store the duration of the music file

    // Load the font
    TTF_Font *font = TTF_OpenFont("font.ttf", 24);
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

    int currentVolume = MIX_MAX_VOLUME / 2; // Set initial volume to half of the maximum volume

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
                    const char *filepath = tinyfd_openFileDialog("Choose Music File", "", 0, nullptr, nullptr, 0);

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
                            musicDuration = Mix_MusicDuration(music); // Get the duration of the music file
                            if (musicDuration <= 0)
                            {
                                std::cout << "Failed to get music duration: " << Mix_GetError() << std::endl;
                            }
                            else
                            {
                                if (Mix_PlayMusic(music, 0) == -1)
                                {
                                    std::cout << "Failed to play music: " << Mix_GetError() << std::endl;
                                }
                                else
                                {
                                    isMusicPlaying = true;
                                    startTime = SDL_GetTicks() / 1000;
                                }
                            }
                        }

                        SDL_free((void *)filepath);
                    }
                }
                // Check if the mouse click is inside the volume slider area
                SDL_Rect volumeSliderRect = {(WIDTH - 200) / 2, HEIGHT - 250, 200, 20};
                if (isPointInRect(mouseX, mouseY, volumeSliderRect))
                {
                    // Calculate the new volume based on the mouse position within the slider
                    int sliderPosition = mouseX - volumeSliderRect.x;
                    int sliderMaxPosition = volumeSliderRect.w;
                    currentVolume = (sliderPosition * MIX_MAX_VOLUME) / sliderMaxPosition;

                    // Set the new volume
                    Mix_VolumeMusic(currentVolume);
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
        SDL_Color textColor = {255, 255, 255, 255}; // White color
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, "Choose file", textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        int textX = buttonRect.x + (buttonRect.w - textWidth) / 2;  // Center horizontally
        int textY = buttonRect.y + (buttonRect.h - textHeight) / 2; // Center vertically
        SDL_Rect textRect = {textX, textY, textWidth, textHeight};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        /// Render the volume slider
        SDL_Rect volumeSliderRect = {(WIDTH - 200) / 2, HEIGHT - 250, 200, 30};
        SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255); // Purple color
        SDL_RenderFillRect(renderer, &volumeSliderRect);

        // Render the volume text
        SDL_Surface *volumeSurface = TTF_RenderText_Solid(font, "Volume", textColor);
        SDL_Texture *volumeTexture = SDL_CreateTextureFromSurface(renderer, volumeSurface);
        int volumeWidth = volumeSurface->w;
        int volumeHeight = volumeSurface->h;
        int volumeX = volumeSliderRect.x - volumeWidth - 10;                        // Position the text to the left of the slider with a margin of 10 pixels
        int volumeY = volumeSliderRect.y + (volumeSliderRect.h - volumeHeight) / 2; // Center the text vertically
        SDL_Rect volumeRect = {volumeX, volumeY, volumeWidth, volumeHeight};
        SDL_RenderCopy(renderer, volumeTexture, NULL, &volumeRect);
        SDL_FreeSurface(volumeSurface);
        SDL_DestroyTexture(volumeTexture);

        // Calculate the position of the volume slider handle
        int sliderPosition = (currentVolume * volumeSliderRect.w) / MIX_MAX_VOLUME;
        SDL_Rect volumeSliderHandleRect = {volumeSliderRect.x + sliderPosition - 5, volumeSliderRect.y - 5, 10, 40};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
        SDL_RenderFillRect(renderer, &volumeSliderHandleRect);

        // Render the music progress
        if (isMusicPlaying && Mix_PlayingMusic())
        {
            int currentTime = SDL_GetTicks() / 1000 - startTime;
            std::string progressText = "Time: " + formatTime(currentTime) + " / " + formatTime(musicDuration);

            SDL_Surface *progressSurface = TTF_RenderText_Solid(font, progressText.c_str(), textColor);
            SDL_Texture *progressTexture = SDL_CreateTextureFromSurface(renderer, progressSurface);
            int progressWidth = progressSurface->w;
            int progressHeight = progressSurface->h;
            int progressX = (WIDTH - progressWidth) / 2;
            int progressY = 50;
            SDL_Rect progressRect = {progressX, progressY, progressWidth, progressHeight};
            SDL_RenderCopy(renderer, progressTexture, NULL, &progressRect);
            SDL_FreeSurface(progressSurface);
            SDL_DestroyTexture(progressTexture);
        }

        SDL_RenderPresent(renderer);
    }

    // Clean up resources
    if (music != nullptr)
    {
        Mix_FreeMusic(music);
    }

    TTF_CloseFont(font);
    TTF_Quit();

    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
