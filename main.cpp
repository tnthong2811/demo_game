#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm> 
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int TOWER_HEIGHT = 60;
const int FALL_SPEED = 7;

struct Tower {
    int x, y, width;
    int speed; 
    bool isFalling;
};

vector<Tower> towers;
bool gameOver = false;
bool quit = false;
int towerspeed = 5;
void addTower(const int width = 300) { 
    int position = rand() % 3;
    int x;
    switch (position) {
        case 0: 
            x = 1;
            break;
        case 1: 
            x = SCREEN_WIDTH / 2 - width / 2;
            break;
        case 2:
            x = SCREEN_WIDTH - width - 1;
            break;
    }
    Tower tower = {x, SCREEN_HEIGHT /2 - TOWER_HEIGHT - 20, width, towerspeed, false}; 
    towers.push_back(tower);
}

int score = 0;
string message; 
int messageTime = 0; 
const int messageDuration = 500; 
bool checkCollision(Tower& fallingTower) {
    if (towers.size() < 2) return false;

    Tower& belowTower = towers[towers.size() - 2];

    bool collisionX = fallingTower.x + fallingTower.width > belowTower.x && fallingTower.x < belowTower.x + belowTower.width;
    bool collisionY = fallingTower.y + TOWER_HEIGHT >= belowTower.y;

    if (collisionX && collisionY) {
        int left = max(fallingTower.x, belowTower.x);
        int right = min(fallingTower.x + fallingTower.width, belowTower.x + belowTower.width);
        int sc = (right - left) / 10;
        fallingTower.width = right - left;
        fallingTower.x = left;
        score += ceil(sc);
        double widthRatio = (double)fallingTower.width / belowTower.width;
        if (widthRatio >= 0.96) {
            message = "Perfectly!";
        } else if (widthRatio >= 0.9) {
            message = "Excellent!";
        } else {
            message = ""; 
        }
        messageTime = SDL_GetTicks(); 
        return true;
    }
    return false;
}

void renderMessage(SDL_Renderer* renderer, TTF_Font* font) {
    if (!message.empty() && SDL_GetTicks() - messageTime < messageDuration) {
        SDL_Color color = {200, 100, 30, 255}; 
        SDL_Surface* surface = TTF_RenderText_Solid(font, message.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {SCREEN_WIDTH / 2 - surface->w / 2, 160, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

int totalFallenTowersHeight = 0; 
int gold = 0;
bool showNotEnoughGoldMessage = false; 
void updateTowerPosition() {
    if (!towers.empty() && !gameOver) {
        Tower& currentTower = towers.back();
        if (!currentTower.isFalling) {
            if (currentTower.x <= 0 || currentTower.x + currentTower.width >= SCREEN_WIDTH) {
                currentTower.speed = -currentTower.speed;
            }
            currentTower.x += currentTower.speed;
        } else {
            currentTower.y += FALL_SPEED;
            if ((towers.size() > 1 && currentTower.y >= towers[towers.size() - 2].y - TOWER_HEIGHT)) {
                currentTower.isFalling = false; 
                if (towers.size() == 1 || (currentTower.y < SCREEN_HEIGHT - TOWER_HEIGHT && checkCollision(currentTower))) {
                    totalFallenTowersHeight += TOWER_HEIGHT;
                    gold += 20;
                    if (totalFallenTowersHeight >= SCREEN_HEIGHT / 2 - TOWER_HEIGHT) {
                        for (auto& tower : towers) {
                            tower.y += TOWER_HEIGHT; 
                        }
                        totalFallenTowersHeight -= TOWER_HEIGHT; 
                    }
                    addTower(currentTower.width);
                }  else {
                    gameOver = true;
                }
            }
        }
    }
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* tempSurface = IMG_Load(filePath);
    if (tempSurface == nullptr) {
        cerr << "Unable to load background image. SDL Error: " << SDL_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* Texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface); 
    
    if (Texture == nullptr) {
        cerr << "Unable to create texture from surface. SDL Error: " << SDL_GetError() << endl;
    }
    return Texture;
}

vector<SDL_Texture*> coinTextures;
void loadCoinTextures(SDL_Renderer* renderer) {
    vector<string> imagePath = {"coin_0.png", "coin_1.png", "coin_2.png", "coin_3.png", "coin_4.png", "coin_5.png", "coin_6.png", "coin_7.png"};
    for (const auto& path : imagePath) {
        SDL_Texture* texture = loadTexture(renderer, path.c_str());
            coinTextures.push_back(texture);
    }
}
int currentFrame = 0;
int t = 0;
void renderCoin(SDL_Renderer* renderer, int x, int y) {
    SDL_Rect dstRect = {x, y, 25, 25};
    SDL_RenderCopy(renderer, coinTextures[currentFrame], NULL, &dstRect);
    if(t % 4 == 0) {
    currentFrame = (currentFrame + 1) % coinTextures.size();
    }
    t++;
}

void renderTowers(SDL_Renderer* renderer) {
    SDL_Texture* towerTexture ;
    for (const auto& tower : towers) {
        SDL_Rect towerRect = {tower.x, tower.y, tower.width, TOWER_HEIGHT};
    if(tower.width == 300) {
        towerTexture = loadTexture(renderer, "tower.png");
    } else if(tower.width >= 250 && tower.width < 300) {
        towerTexture = loadTexture(renderer, "tower3.png");
    } else if(tower.width >= 200 && tower.width < 250) {
        towerTexture = loadTexture(renderer, "tower4.png");
    } else if(tower.width >= 150 && tower.width < 200) {
        towerTexture = loadTexture(renderer, "tower5.png");
    } else if(tower.width >= 100 && tower.width < 150) {
        towerTexture = loadTexture(renderer, "tower6.png");
    } else if(tower.width >= 85 && tower.width < 100) {
        towerTexture = loadTexture(renderer, "tower7.png");
    } else {
        towerTexture = loadTexture(renderer, "tower2.png");
    }
        SDL_RenderCopy(renderer, towerTexture, NULL, &towerRect);
    }
}

void showInitialScreen(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color color = {255, 210, 0, 255}; 
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Tower Building", color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect;
    rect.x = (SCREEN_WIDTH - surface->w) / 2; 
    rect.y = (SCREEN_HEIGHT - surface->h) / 2 - 95; 
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_FreeSurface(surface); 
    SDL_RenderCopy(renderer, texture, NULL, &rect); 
    SDL_DestroyTexture(texture);
}

bool handleButtonEvents(SDL_Renderer* renderer, SDL_Event& e, SDL_Rect& ButtonRect, SDL_Texture* ButtonTexture) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    const int normalWidth = 100;
    const int normalHeight = 40;
    const int largeWidth = 110;
    const int largeHeight = 45;

    bool isMouseOverButton = mouseX > ButtonRect.x && mouseX < ButtonRect.x + ButtonRect.w &&
                             mouseY > ButtonRect.y && mouseY < ButtonRect.y + ButtonRect.h;

    if (isMouseOverButton) {
        ButtonRect.w = largeWidth;
        ButtonRect.h = largeHeight; 
    } else {
        ButtonRect.w = normalWidth;
        ButtonRect.h = normalHeight;
    }
    SDL_RenderCopy(renderer, ButtonTexture, NULL, &ButtonRect);
    if (e.type == SDL_MOUSEBUTTONDOWN && isMouseOverButton) {
        return true;
    }
    return false;
}

bool showPlayButtonAndWaitForClick(SDL_Renderer* renderer, SDL_Texture* backgroundTexture, SDL_Texture* playButtonTexture, TTF_Font* font) {

    SDL_Rect playButtonRect;
    playButtonRect.x = (SCREEN_WIDTH - 100) / 2; 
    playButtonRect.y = (SCREEN_HEIGHT - 40) / 2 + 50;
    playButtonRect.w = 100;
    playButtonRect.h = 40;

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
                return false; 
            } else if (handleButtonEvents(renderer, e, playButtonRect, playButtonTexture)) {
                    return true; 
            }
        }
        SDL_RenderClear(renderer);
        if (backgroundTexture != nullptr) {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }
        if (playButtonTexture != nullptr) {
            SDL_RenderCopy(renderer, playButtonTexture, NULL, &playButtonRect);
        }
        if (playButtonTexture != nullptr) {
            showInitialScreen(renderer, font);
        }
        SDL_RenderPresent(renderer);
        }
    }

void showScore(SDL_Renderer* renderer, TTF_Font* font, int score, int x, int y) {
    SDL_Color color = {255, 255, 255, 255}; 
    string scoreText = "Score: " + to_string(score);
    SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect;
    rect.x = x;
    rect.y = y; 
    rect.w = surface->w; 
    rect.h = surface->h; 

    SDL_FreeSurface(surface); 
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture); 
}

void showGoldAndMessage(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color color = {255, 255, 255, 255}; 
    string goldText = to_string(gold) ;
    SDL_Surface* surfaceGold = TTF_RenderText_Solid(font, goldText.c_str(), color);
    SDL_Texture* textureGold = SDL_CreateTextureFromSurface(renderer, surfaceGold);
    SDL_Rect rectGold = {10, 35, surfaceGold->w, surfaceGold->h}; 
    SDL_RenderCopy(renderer, textureGold, NULL, &rectGold);
    SDL_FreeSurface(surfaceGold);
    SDL_DestroyTexture(textureGold);
    if (showNotEnoughGoldMessage) {
        SDL_Color color1 = {255, 0, 0, 255}; 
        string message = "Not enough coins!";
        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, message.c_str(), color1);
        SDL_Texture* textureMessage = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_Rect rectMessage = {SCREEN_WIDTH / 2 - surfaceMessage->w / 2, 40, surfaceMessage->w + 10, surfaceMessage->h + 10};
        SDL_RenderCopy(renderer, textureMessage, NULL, &rectMessage);
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(textureMessage);
        // Hiển thị thông báo trong 2 giây
        SDL_RenderPresent(renderer);
        SDL_Delay(2000);
        showNotEnoughGoldMessage = false;
    }
}

void showGround(SDL_Renderer* renderer) {
    SDL_Texture* ground = loadTexture(renderer, "ground.png");
    SDL_Texture* streetlight = loadTexture(renderer, "streetlight.png");
    SDL_Texture* tree = loadTexture(renderer, "tree1.png");
    SDL_Texture* bench = loadTexture(renderer, "bench.png");
    SDL_Texture* water = loadTexture(renderer, "water1.png");
    SDL_Texture* bench2 = loadTexture(renderer, "bench2.png");
    SDL_Rect groundRect = {0, SCREEN_HEIGHT - 20 , 600, 20};
    SDL_Rect streetlightRect = {100, SCREEN_HEIGHT - 20 - 100, 30, 100};
    SDL_Rect treeRect = {460, SCREEN_HEIGHT - 20 - 60, 45, 60};
    SDL_Rect benchRect = {25, SCREEN_HEIGHT - 20 - 30, 50, 30};
    SDL_Rect waterRect = {500, SCREEN_HEIGHT - 20 - 20, 14, 20};
    SDL_Rect bench2Rect = {540, SCREEN_HEIGHT - 20 - 30, 55, 30};
    if(towers.size() < 6) {
        SDL_RenderCopy(renderer, ground, NULL, &groundRect);
        SDL_RenderCopy(renderer, streetlight, NULL, &streetlightRect);
        SDL_RenderCopy(renderer, tree, NULL, &treeRect);
        SDL_RenderCopy(renderer, bench, NULL, &benchRect);
        SDL_RenderCopy(renderer, water, NULL, &waterRect);
        SDL_RenderCopy(renderer, bench2, NULL, &bench2Rect);

    }
}

void saveGold(int gold) {
    ofstream file("gold.txt");
    if (file.is_open()) {
        file << gold;
        file.close();
    } else {
        cerr << "Unable to open file to save gold." << endl;
    }
}

int loadGold() {
    ifstream file("gold.txt");
    int gold = 0;
    if (file.is_open()) {
        file >> gold;
        file.close();
    } else {
        cerr << "Unable to open file to load gold." << endl;
    }
    return gold;
}

void saveHighScore(int score) {
    ofstream file("highscore.txt");
    file << score;
    file.close();
}

int loadHighScore() {
    ifstream file("highscore.txt");
    int highScore;
    if (file >> highScore) {
        return highScore;
    }
    return 0; 
}

void updateHighScore(int score) {
    int highScore = loadHighScore();
    if (score > highScore) {
        saveHighScore(score);
    }
}

void resetGame() {
    towers.clear(); 
    Tower baseTower = {SCREEN_WIDTH / 2 - 300 / 2, SCREEN_HEIGHT - TOWER_HEIGHT - 20, 300, false};
    towers.push_back(baseTower); 
    score = 0;
    gameOver = false; 
    totalFallenTowersHeight = 0; 
    addTower(); 
}

void Continue() {
    if (gold >= 1000) {
        gold -= 1000;
        towers.pop_back();
        towers.pop_back();
        gameOver = false;
        Tower continueTower = {SCREEN_WIDTH / 2 -  150 / 2, SCREEN_HEIGHT - totalFallenTowersHeight - TOWER_HEIGHT - 20, 150, false};
        towers.push_back(continueTower);
        addTower(150);
        showNotEnoughGoldMessage = false;
    } else {
        showNotEnoughGoldMessage = true; 
    }
}

void showGameOver(SDL_Renderer* renderer, TTF_Font* font,SDL_Texture* menuTexture,  SDL_Rect menuRect) {
    SDL_Texture* gameOverTexture = loadTexture(renderer, "Gameover.bmp");
    int highScore = loadHighScore();
    SDL_Color color = {255, 0, 0, 0}; 
    string highScoreText = "High Score: " + to_string(highScore);
    SDL_Surface* surfaceHighScore = TTF_RenderText_Solid(font, highScoreText.c_str(), color);
    SDL_Texture* textureHighScore = SDL_CreateTextureFromSurface(renderer, surfaceHighScore);
    SDL_Rect rectHighScore = {SCREEN_WIDTH / 2 - surfaceHighScore->w / 2, SCREEN_HEIGHT / 2 + 100, surfaceHighScore->w, surfaceHighScore->h}; // Dịch xuống một chút so với điểm số
    SDL_Event e;        
    handleButtonEvents(renderer, e, menuRect, menuTexture);
    SDL_RenderCopy(renderer, gameOverTexture, NULL, NULL);
    SDL_RenderCopy(renderer, textureHighScore, NULL, &rectHighScore);
    SDL_RenderCopy(renderer, menuTexture, NULL, &menuRect);
    SDL_FreeSurface(surfaceHighScore);
    SDL_DestroyTexture(textureHighScore);
    SDL_DestroyTexture(gameOverTexture);
}

int main(int argc, char* args[]) {
    SDL_Window* window = SDL_CreateWindow("Tower Building", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG)) {
        cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
        SDL_Quit();
        return -1;
    }
    if (TTF_Init() == -1) {
        cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return -1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
    SDL_Quit();
    return -1;
    }
    Mix_Music *backgroundMusic = Mix_LoadMUS("background.mp3");
    if (backgroundMusic == NULL) {
        cout << "Failed to load background music! SDL_mixer Error: " << Mix_GetError() << endl;
    } else {
    Mix_PlayMusic(backgroundMusic, -1);
    }
    gold = loadGold();
    loadCoinTextures(renderer);
    SDL_Rect menuRect = {SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 60 , 100, 40};
    SDL_Texture* backgroundTexture = loadTexture(renderer, "background.png");
    SDL_Texture* playButtonTexture = loadTexture(renderer, "play.png");
    SDL_Texture* menuTexture = loadTexture(renderer, "quit.png");
    TTF_Font* font = TTF_OpenFont("Pixel.ttf", 16);
    TTF_Font* font1 = TTF_OpenFont("Pixel.ttf", 26);
    TTF_Font* highscore = TTF_OpenFont("Pixel.ttf", 32);
    TTF_Font* name = TTF_OpenFont("Pixel.ttf", 40);
    Tower baseTower = {SCREEN_WIDTH / 2 - 300 / 2, SCREEN_HEIGHT - TOWER_HEIGHT - 20, 300, false};
    towers.push_back(baseTower);
    
    bool gameStarted = showPlayButtonAndWaitForClick(renderer, backgroundTexture, playButtonTexture, name);
    if (!gameStarted) {
        return 0;
    }
    addTower();

    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN :  
                    switch (e.key.keysym.sym) {
                    case SDLK_SPACE:
                        if (!towers.empty() && !gameOver) {
                            towers.back().isFalling = true;
                        }
                        break;
                    }
                    if (gameOver) {
                        switch (e.key.keysym.sym) {
                            case SDLK_RETURN:
                                resetGame();
                                break; 
                            case SDLK_c:
                                Continue();
                                break;    
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN: 
                    if (gameOver && handleButtonEvents(renderer, e, menuRect, menuTexture)) {    
                        showPlayButtonAndWaitForClick(renderer, backgroundTexture, playButtonTexture, name);
                        resetGame();
                    }
                    break;
                }
        }

        updateTowerPosition();
        SDL_RenderClear(renderer);
        if (!gameOver) {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL); 
            showGround(renderer);
            renderTowers(renderer);
            showScore(renderer, font, score, 10, 10);
            renderCoin(renderer, 70, 35);
            showGoldAndMessage(renderer, font);
             renderMessage(renderer, font1); 
        } else {
            updateHighScore(score);
            showGameOver(renderer, highscore, menuTexture, menuRect);
            
            showScore(renderer, font1, score, (SCREEN_WIDTH - 200) / 2, 70);
            renderCoin(renderer, 70, 35);
            showGoldAndMessage(renderer, font);
        } 

        SDL_RenderPresent(renderer);
        SDL_Delay(1000/60); 

    }
    saveGold(gold);
    coinTextures.clear();
    TTF_CloseFont(font);
    TTF_Quit();
    Mix_FreeMusic(backgroundMusic);
   
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(backgroundTexture);
    SDL_Quit();
    return 0;
}