#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <cmath>
#include <deque>
#include <vector>
#include <algorithm>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

const int GAME_WINDOW_HEIGHT = 720;
const int GAME_WINDOW_WIDTH = 1280;

enum GameState {
    MENU, GAME, GAMEOVER, RESET
};

enum State {
    idle,
    up,
    down,
    left,
    right
};

class Snake {
    public: 
    int posx;
    int posy;
    static const int snake_width = 30;
    static const int snake_height = 30;
    State state;
    int speed;
    std::deque<SDL_Rect> tail;
    int size = 1;

    Snake() {
        posx = GAME_WINDOW_WIDTH / 2;
        posy = GAME_WINDOW_HEIGHT / 2;
        state = idle;
        speed = 1;
        std::deque<SDL_Rect> tail;
        int size = 1;
    }

    SDL_Rect getRect() const {
    SDL_Rect rect;
    rect.x = posx;
    rect.y = posy;
    rect.w = snake_width;
    rect.h = snake_height;
    return rect;
    }
};

class Food {
    public: 
    int posx;
    int posy;
    static const int food_width = 20;
    static const int food_height = 20;

    Food() {
    posx = (food_width/2) + (rand() % (GAME_WINDOW_WIDTH - food_width*2));
    posy = (food_height/2) + (rand() % (GAME_WINDOW_HEIGHT - food_height*2));
    }

    SDL_Rect getRect() const {
    SDL_Rect rect;
    rect.x = posx;
    rect.y = posy;
    rect.w = food_width;
    rect.h = food_height;
    return rect;
    }
};

void DrawFood(int x, int y, SDL_Renderer * renderer) {
    SDL_Rect drawFood;
    drawFood.x = x;
    drawFood.y = y;
    drawFood.w = 20;
    drawFood.h = 20;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, & drawFood);
}

bool isOutOfBounds(const SDL_Rect& rect) {
return (rect.x < 0 || rect.x + rect.w > GAME_WINDOW_WIDTH ||
rect.y < 0 || rect.y + rect.h > GAME_WINDOW_HEIGHT);
}

bool checkCollision(const SDL_Rect& rect1, const SDL_Rect& rect2) {
return (rect1.x < rect2.x + rect2.w &&
rect1.x + rect1.w > rect2.x &&
rect1.y < rect2.y + rect2.h &&
rect1.y + rect1.h > rect2.y);
}

void RenderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, const SDL_Color& color, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstrect;
    dstrect.x = x;
    dstrect.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &dstrect.w, &dstrect.h);

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

std::vector<Mix_Chunk*> sounds;
std::vector<Mix_Music*> music;

int loadMusic(const char* filename)
{
    Mix_Music *m = NULL;
    m = Mix_LoadMUS(filename);
    if(m == NULL)
    {
        std::cout << "failed to load \n";
        return 1;
    }
    music.push_back(m);
    return music.size()-1;
}

int loadSound(const char* filename)
{
    Mix_Chunk *s = NULL;
    s = Mix_LoadWAV(filename);
    if(s == NULL)
    {
        std::cout << "failed to load \n";
        return -1;
    }
    sounds.push_back(s);
    return sounds.size() - 1;
}

int volume;
void setVolume(int v)
{
    volume = (MIX_MAX_VOLUME * v) / 1000;
}

int playMusic(int m)
{
    if (Mix_PlayingMusic() == 0)
    {
        Mix_Volume(1, volume);
        Mix_PlayMusic(music[m], -1);
    }
    return 0;
}

int playSound(int s) {
    // Stop the currently playing sound
    Mix_HaltChannel(-1);

    if (sounds[s] != nullptr) {
        int channel = Mix_PlayChannel(-1, sounds[s], 0);
        if (channel == -1) {
            // Failed to play sound
            return -1;
        }
    }
    return 0;
}

int initMixer()
{
    Mix_Init(MIX_INIT_MP3);
    SDL_Init(SDL_INIT_AUDIO);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cout<<"mixer couldnt init \n";
        return -1;
    }
    setVolume(1);
    return 0;   
}

void quitMixer()
{
    for (int s=0; s < sounds.size(); s++)
    {
        Mix_FreeChunk(sounds[s]);
        sounds[s] = NULL;
    }
    for (int s = 0; s < music.size(); s++)
    {
        Mix_FreeMusic(music[s]);
        music[s] = NULL;
    }
    sounds.clear();
    music.clear();
    Mix_Quit();
}

void togglePlay()
{
    if(Mix_PausedMusic() == 1)
    {
        Mix_ResumeMusic();
    }
    else{
        Mix_PauseMusic();
    }
}

int main(int argc, char ** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    initMixer();
    int sound = loadSound("src/misc/hold.wav");
    int song = loadMusic("src/misc/lonely.mp3");
    playMusic(song);

    int highscore = 0;
    int score = 0;
    int points = 0;

    SDL_Window * window = SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    std::string scoreHeader = "Score: ";
    std::string scoreString = std::to_string(score);
    std::string scoreFinal = scoreHeader + scoreString;

    TTF_Font * font = TTF_OpenFont("src/misc/PlatNomor-WyVnn.ttf", 25);
    SDL_Color color = { 255, 255, 255 };

    SDL_Surface * surface = TTF_RenderText_Solid(font,
 scoreFinal.c_str(), color);
    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstrect = { 0, 0, texW, texH };

    GameState gameState = MENU;
    Snake snake;
    Food food;

    bool isRunning = true;
    SDL_Event event;

    while ((isRunning)) {
        
        SDL_RenderClear(renderer);
        playMusic(song);

        while (SDL_PollEvent( & event)) {
            if (gameState == MENU)
            {
                switch(event.type)
                {
                    case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        isRunning = false;
                        break;
                    case SDL_QUIT:
                        isRunning = false;
                        break;
                    case SDLK_SPACE:
                        gameState = GAME;
                        break;
                    }
                }
            }
            else if (gameState == GAME)
            {
                switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        isRunning = false;
                        break;
                    case SDL_QUIT:
                        isRunning = false;
                        break;
                    case SDLK_DOWN:
                        if(snake.state != up)
                        snake.state = down;
                        break;
                    case SDLK_UP:
                        if(snake.state != down)
                        snake.state = up;
                        break;
                    case SDLK_LEFT:
                        if(snake.state != right)
                        snake.state = left;
                        break;
                    case SDLK_RIGHT:
                        if(snake.state != left)
                        snake.state = right;
                        break;
                    default:
                        break;
                    }
                }
            }
            else if (gameState == GAMEOVER)
            {
                switch(event.type)
                {
                    case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        isRunning = false;
                        break;
                    case SDL_QUIT:
                        isRunning = false;
                        break;
                    case SDLK_SPACE:
                        gameState = RESET;
                        break;
                    }
                }
            }
            else if (gameState == RESET)
            {
                score = 0;
                points = 0;
                snake.size = 1;
                snake.posx = GAME_WINDOW_WIDTH / 2;
                snake.posy = GAME_WINDOW_HEIGHT / 2;
                snake.state = idle;
                snake.speed = 1;
                food.posx = (food.food_width/2) + (rand() % (GAME_WINDOW_WIDTH - food.food_width*2));
                food.posy = (food.food_height/2) + (rand() % (GAME_WINDOW_HEIGHT - food.food_height*2));
                gameState = MENU;
                break;
            }
            break;
        }
        if (gameState == MENU) {
            // Render the menu screen
            RenderText(renderer, font, "Snake Game", color, GAME_WINDOW_WIDTH / 2 - 60, GAME_WINDOW_HEIGHT / 2 - 20);
            RenderText(renderer, font, "Press Space to Start", color, GAME_WINDOW_WIDTH / 2 - 110, GAME_WINDOW_HEIGHT / 2 + 20);
            RenderText(renderer, font, "Press Esc to quit", color, GAME_WINDOW_WIDTH / 2 - 90, GAME_WINDOW_HEIGHT / 2 + 100);
        }
        else if (gameState == GAME)
        {
            switch (snake.state) {
            case up:
                snake.posy -= snake.speed;
                break;

            case down:
                snake.posy += snake.speed;
                break;

            case left:
                snake.posx -= snake.speed;
                break;

            case right:
                snake.posx += snake.speed;
                break;

            case idle:
                break;
            }
            SDL_Rect snakeRect = snake.getRect();
            SDL_Rect foodRect = food.getRect();

            if (checkCollision(snakeRect, foodRect)) {
            food.posx = (food.food_width/2) + (rand() % (GAME_WINDOW_WIDTH - food.food_width*2));
            food.posy = (food.food_height/2) + (rand() % (GAME_WINDOW_HEIGHT - food.food_height*2));
            points += 1;
            snake.size += 10;
            score += 1;
            playSound(sound);
            }
            if (points == 5) {
                snake.speed += 1;
                points = 0;
            }
            
            if (isOutOfBounds(snakeRect)) {
                playSound(sound);
                gameState = GAMEOVER;
            }

            std::for_each(snake.tail.begin(), snake.tail.end(), [&](auto& snake_segment)
            {
                if ((snake.posx == snake_segment.x || -snake.posx == -snake_segment.x) && (snake.posy == snake_segment.y || -snake.posy == -snake_segment.y) && snake.size > 1 )
                {
                    playSound(sound);
                    gameState = GAMEOVER;
                }
            });

            snake.tail.push_front(snake.getRect());

            while(snake.tail.size() > snake.size)
            {
                snake.tail.pop_back();
            }
            
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            std::for_each(snake.tail.begin(), snake.tail.end(), [&](auto& snake_segment)
            {
                SDL_RenderFillRect(renderer, &snake_segment);
            });

            
            DrawFood(food.posx, food.posy, renderer);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            std::string scoreString = std::to_string(score);
            std::string scoreFinal = scoreHeader + scoreString;
            SDL_Surface* surface = TTF_RenderText_Solid(font, scoreFinal.c_str(), color);
            SDL_DestroyTexture(texture); // Destroy previous texture
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
            dstrect.w = texW;
            dstrect.h = texH;
            SDL_RenderCopy(renderer, texture, NULL, &dstrect);
            SDL_FreeSurface(surface);
        }
        else if (gameState == GAMEOVER) {
            if (score > highscore)
            {
                highscore = score;
            }
            // Render the game over screen
            RenderText(renderer, font, "Game Over!", color, GAME_WINDOW_WIDTH / 2 - 60, GAME_WINDOW_HEIGHT / 2 - 20);
            RenderText(renderer, font, "Press Space to Restart", color, GAME_WINDOW_WIDTH / 2 - 110, GAME_WINDOW_HEIGHT / 2 + 20);
            RenderText(renderer, font, "Score: " + std::to_string(score), color, GAME_WINDOW_WIDTH / 2 - 40, GAME_WINDOW_HEIGHT / 2 + 60);
            RenderText(renderer, font, "High Score: " + std::to_string(highscore), color, GAME_WINDOW_WIDTH / 2 - 60, GAME_WINDOW_HEIGHT / 2 + 100);
            RenderText(renderer, font, "Press Esc to quit", color, GAME_WINDOW_WIDTH / 2 - 90, GAME_WINDOW_HEIGHT / 2 + 140);

        }

        SDL_RenderPresent(renderer);

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);	
    TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    quitMixer();
    SDL_Quit();
    

    return 0;
}
