#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <string>
#include <queue>
#include <filesystem>
#include <Tiny_File_Dialogs/tinyfiledialogs.h>

const int WIDTH = 1920, HEIGHT = 1080;
std::queue<std::string> songQueue;
std::string currentFilename;
bool quit = false;
bool isMusicPlaying = false;
bool isMusicPaused = false;
int startTime = 0;
int pauseTime = 0;

Mix_Music *music = nullptr;
int musicDuration = 0;
const char *albumTag = nullptr;
const char *artistTag = nullptr;
const char *titleTag = nullptr;

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

void playNextSong()
{
    if (!songQueue.empty())
    {
        std::string filepath = songQueue.front();
        songQueue.pop();

        if (music != nullptr)
        {
            Mix_FreeMusic(music);
            music = nullptr;
        }

        music = Mix_LoadMUS(filepath.c_str());
        if (music == nullptr)
        {
            std::cout << "Failed to load music: " << Mix_GetError() << std::endl;
        }
        else
        {
            musicDuration = Mix_MusicDuration(music);
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
                    albumTag = Mix_GetMusicAlbumTag(music);
                    if (albumTag == nullptr || strlen(albumTag) < 2)
                    {
                        albumTag = "Unknown";
                    }

                    artistTag = Mix_GetMusicArtistTag(music);
                    if (artistTag == nullptr || strlen(artistTag) < 2)
                    {
                        artistTag = "Unknown";
                    }

                    titleTag = Mix_GetMusicTitle(music);
                    if (titleTag == nullptr || strlen(titleTag) < 2)
                    {
                        titleTag = "Unknown";
                    }

                    currentFilename = std::filesystem::path(filepath).filename().string(); // Extract the filename
                    isMusicPlaying = true;
                    startTime = SDL_GetTicks() / 1000;
                }
            }
        }
    }
}

void addToQueue(const char *filepath)
{
    std::string songPath(filepath);
    songQueue.push(songPath);

    if (!isMusicPlaying)
    {
        playNextSong();
    }
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("AudioFlow", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
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

    // Initialize SDL2_image
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        std::cout << "SDL2_image could not initialize: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load the background image
    SDL_Surface *backgroundSurface = IMG_Load("background.png");
    if (backgroundSurface == nullptr)
    {
        std::cout << "Failed to load background image: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
    if (backgroundTexture == nullptr)
    {
        std::cout << "Failed to create background texture: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load the font
    TTF_Font *font = TTF_OpenFont("font.ttf", 24);
    if (font == nullptr)
    {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int currentVolume = MIX_MAX_VOLUME / 4; // Set initial volume to 25%
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
                SDL_Rect buttonRect = {(WIDTH - 200) / 2, HEIGHT - 100, 200, 50};
                if (isPointInRect(mouseX, mouseY, buttonRect))
                {
                    // Open file dialog to choose a music file
                    const char *filepath = tinyfd_openFileDialog("Choose Music File", "", 0, nullptr, nullptr, 0);

                    if (filepath != nullptr)
                    {
                        isMusicPaused = false;
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
                            albumTag = Mix_GetMusicAlbumTag(music);
                            if (albumTag == nullptr || strlen(albumTag) < 2)
                            {
                                albumTag = "Unknown";
                            }

                            artistTag = Mix_GetMusicArtistTag(music);
                            if (artistTag == nullptr || strlen(artistTag) < 2)
                            {
                                artistTag = "Unknown";
                            }

                            titleTag = Mix_GetMusicTitle(music);
                            if (titleTag == nullptr || strlen(titleTag) < 2)
                            {
                                titleTag = "Unknown";
                            }
                            musicDuration = Mix_MusicDuration(music);                              // Get the duration of the music file
                            currentFilename = std::filesystem::path(filepath).filename().string(); // Extract the filename
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

                SDL_Rect pauseButtonRect = {(WIDTH - 200) / 2, (HEIGHT - 200), 200, 50};
                if (isPointInRect(mouseX, mouseY, pauseButtonRect))
                {
                    if (isMusicPlaying)
                    {
                        if (isMusicPaused)
                        {
                            // Resume the music
                            Mix_ResumeMusic();
                            isMusicPaused = false;

                            // Update the start time by subtracting the paused time
                            startTime += (SDL_GetTicks() / 1000 - pauseTime);
                        }
                        else
                        {
                            // Pause the music
                            Mix_PauseMusic();
                            isMusicPaused = true;

                            // Store the current time as the paused time
                            pauseTime = SDL_GetTicks() / 1000;
                        }
                    }
                }
                // Check if the mouse click is inside the volume slider area
                SDL_Rect volumeSliderRect = {(WIDTH - 200) / 2, HEIGHT - 400, 200, 20};
                if (isPointInRect(mouseX, mouseY, volumeSliderRect))
                {
                    // Calculate the new volume based on the mouse position within the slider
                    int sliderPosition = mouseX - volumeSliderRect.x;
                    int sliderMaxPosition = volumeSliderRect.w;
                    currentVolume = (sliderPosition * MIX_MAX_VOLUME) / sliderMaxPosition;

                    // Set the new volume
                    Mix_VolumeMusic(currentVolume);
                }

                SDL_Rect queueButtonRect = {(WIDTH - 200) / 2, HEIGHT - 300, 200, 50};
                if (isPointInRect(mouseX, mouseY, queueButtonRect))
                {
                    // Open file dialog to choose a music file
                    const char *filepath = tinyfd_openFileDialog("Choose Music File", "", 0, nullptr, nullptr, 0);

                    if (filepath != nullptr)
                    {
                        addToQueue(filepath);
                        // Don't use SDL_free for filepath, as it wasn't allocated by SDL_malloc
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the background
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        // Render the button
        SDL_Rect buttonRect = {(WIDTH - 200) / 2, HEIGHT - 100, 200, 50};
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

        // Render the volume slider
        SDL_Rect volumeSliderRect = {(WIDTH - 200) / 2, HEIGHT - 400, 200, 30};
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

        // Render the pause/resume button
        SDL_Rect pauseButtonRect = {(WIDTH - 200) / 2, (HEIGHT - 200), 200, 50};
        SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255); // Purple color
        SDL_RenderFillRect(renderer, &pauseButtonRect);

        // Render the text on the pause/resume button
        std::string pauseButtonText = isMusicPaused ? "Resume" : "Pause";
        SDL_Surface *pauseButtonSurface = TTF_RenderText_Solid(font, pauseButtonText.c_str(), textColor);
        SDL_Texture *pauseButtonTexture = SDL_CreateTextureFromSurface(renderer, pauseButtonSurface);
        int pauseButtonWidth = pauseButtonSurface->w;
        int pauseButtonHeight = pauseButtonSurface->h;
        int pauseButtonX = pauseButtonRect.x + (pauseButtonRect.w - pauseButtonWidth) / 2;  // Center horizontally
        int pauseButtonY = pauseButtonRect.y + (pauseButtonRect.h - pauseButtonHeight) / 2; // Center vertically
        SDL_Rect pauseButtonRenderRect = {pauseButtonX, pauseButtonY, pauseButtonWidth, pauseButtonHeight};
        SDL_RenderCopy(renderer, pauseButtonTexture, NULL, &pauseButtonRenderRect);
        SDL_FreeSurface(pauseButtonSurface);
        SDL_DestroyTexture(pauseButtonTexture);

        // Render the Queue button
        SDL_Rect queueButtonRect = {(WIDTH - 200) / 2, HEIGHT - 300, 200, 50};
        SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255); // Purple color
        SDL_RenderFillRect(renderer, &queueButtonRect);

        // Render the text on the Queue button
        std::string queueButtonText = "Add to Queue";
        SDL_Surface *queueButtonSurface = TTF_RenderText_Solid(font, queueButtonText.c_str(), textColor);
        SDL_Texture *queueButtonTexture = SDL_CreateTextureFromSurface(renderer, queueButtonSurface);
        int queueButtonWidth = queueButtonSurface->w;
        int queueButtonHeight = queueButtonSurface->h;
        int queueButtonX = queueButtonRect.x + (queueButtonRect.w - queueButtonWidth) / 2;  // Center horizontally
        int queueButtonY = queueButtonRect.y + (queueButtonRect.h - queueButtonHeight) / 2; // Center vertically
        SDL_Rect queueButtonRenderRect = {queueButtonX, queueButtonY, queueButtonWidth, queueButtonHeight};
        SDL_RenderCopy(renderer, queueButtonTexture, nullptr, &queueButtonRenderRect);

        // Cleanup
        SDL_FreeSurface(queueButtonSurface);
        SDL_DestroyTexture(queueButtonTexture);

        // Render the music progress
        if (isMusicPlaying && Mix_PlayingMusic() && !isMusicPaused)
        {
            int currentTime = SDL_GetTicks() / 1000 - startTime;
            std::string progressText = formatTime(currentTime) + " / " + formatTime(musicDuration);

            SDL_Surface *progressSurface = TTF_RenderText_Solid(font, progressText.c_str(), textColor);
            SDL_Texture *progressTexture = SDL_CreateTextureFromSurface(renderer, progressSurface);
            int progressWidth = progressSurface->w;
            int progressHeight = progressSurface->h;
            int progressX = (WIDTH - progressWidth) / 2; // Center horizontally
            int progressY = 85;                          
            SDL_Rect progressRect = {progressX, progressY, progressWidth, progressHeight};
            SDL_RenderCopy(renderer, progressTexture, NULL, &progressRect);
            SDL_FreeSurface(progressSurface);
            SDL_DestroyTexture(progressTexture);

            // Render the title tag
            SDL_Surface *titleSurface = TTF_RenderText_Solid(font, titleTag, textColor);
            SDL_Texture *titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
            int titleWidth = titleSurface->w;
            int titleHeight = titleSurface->h;
            int titleX = (WIDTH - titleWidth) / 2;     // Center horizontally
            int titleY = 205; 
            SDL_Rect titleRect = {titleX, titleY, titleWidth, titleHeight};
            SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
            SDL_FreeSurface(titleSurface);
            SDL_DestroyTexture(titleTexture);

            // Render the artist tag
            SDL_Surface *artistSurface = TTF_RenderText_Solid(font, artistTag, textColor);
            SDL_Texture *artistTexture = SDL_CreateTextureFromSurface(renderer, artistSurface);
            int artistWidth = artistSurface->w;
            int artistHeight = artistSurface->h;
            int artistX = (WIDTH - artistWidth) / 2;     // Center horizontally
            int artistY = 325; 
            SDL_Rect artistRect = {artistX, artistY, artistWidth, artistHeight};
            SDL_RenderCopy(renderer, artistTexture, NULL, &artistRect);
            SDL_FreeSurface(artistSurface);
            SDL_DestroyTexture(artistTexture);

            // Render the album tag
            SDL_Surface *albumSurface = TTF_RenderText_Solid(font, albumTag, textColor);
            SDL_Texture *albumTexture = SDL_CreateTextureFromSurface(renderer, albumSurface);
            int albumWidth = albumSurface->w;
            int albumHeight = albumSurface->h;
            int albumX = (WIDTH - albumWidth) / 2;     // Center horizontally
            int albumY = 445; 
            SDL_Rect albumRect = {albumX, albumY, albumWidth, albumHeight};
            SDL_RenderCopy(renderer, albumTexture, NULL, &albumRect);
            SDL_FreeSurface(albumSurface);
            SDL_DestroyTexture(albumTexture);

            // Render the filename text
            SDL_Surface *filenameSurface = TTF_RenderText_Solid(font, currentFilename.c_str(), textColor);
            SDL_Texture *filenameTexture = SDL_CreateTextureFromSurface(renderer, filenameSurface);
            int filenameWidth = filenameSurface->w;
            int filenameHeight = filenameSurface->h;
            int filenameX = (WIDTH - filenameWidth) / 2;     // Center horizontally
            int filenameY = 565; 
            SDL_Rect filenameRect = {filenameX, filenameY, filenameWidth, filenameHeight};
            SDL_RenderCopy(renderer, filenameTexture, NULL, &filenameRect);
            SDL_FreeSurface(filenameSurface);
            SDL_DestroyTexture(filenameTexture);

        }

        if (isMusicPlaying && !Mix_PlayingMusic() && !isMusicPaused)
        {
            playNextSong();
        }

        SDL_RenderPresent(renderer);
    }

    // Clean up resources
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    if (music != nullptr)
    {
        Mix_FreeMusic(music);
    }
    Mix_CloseAudio();
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
