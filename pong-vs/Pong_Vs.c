#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "util.h"



typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float speed;
} ball;

typedef struct {
    SDL_Rect pos;
    int score;
    int speed;
} player;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int CENTER_WIDTH = 5;

const float MAX_ANGLE = 3.142 / 5.0;

const float BALL_MAXSPEED = 8.0f;
const float BALL_ACCELERATE = 1.05f;
const float BALL_INIT_SPEED = 4.0f;
const int BALL_WIDTH = 10;
const int BALL_HEIGHT = 10;

Mix_Chunk* collisionSound;

void Initialise(SDL_Renderer** ren, SDL_Window** win);
void Cleanup(SDL_Renderer** ren, SDL_Window** win);


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

float calc_angle(float y1, float y2, int height) {
    float rely = y1 + height / 2 - y2;
    rely /= height / 2.0;
    return rely * MAX_ANGLE;
}

int main(int argc, char* argv[]) {

    SDL_Event e;
    SDL_Renderer* ren = NULL;
    SDL_Window* win = NULL;

    Uint8 menuState = 0;
    Uint8 KeyPress = 0, KeyPrMess = 0;

//********************************** Init Socket ***************************************************************

     int sockfd, portno;
     int newsockfd = -1;
     socklen_t clilen;
     char Messag[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {

         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);


//**************************************************************************************************************

    printf("Starting SDL Application...\n");

    Initialise(&ren, &win);

    collisionSound = Mix_LoadWAV("assets/bounce.wav");

    int board_width;
    int board_height;
    SDL_Texture* squareTex = IMG_LoadTexture(ren, "img/pong_board.png");
    SDL_QueryTexture(squareTex, NULL, NULL, &board_width, &board_height);

    SDL_Color whiteColor = { 255, 255, 255 };
    SDL_Surface* fpsCounter;

    SDL_Rect b_rect;
    b_rect.w = BALL_HEIGHT;
    b_rect.h = BALL_HEIGHT;

    // Define Players
    player p1;
    player p2;

    // Define Ball
    ball b;
    b.x = SCREEN_WIDTH / 2;
    b.y = SCREEN_HEIGHT / 2;
    b.speed = BALL_INIT_SPEED;
    b.vx = (rand() % 2 == 0) ? BALL_INIT_SPEED : -1 * BALL_INIT_SPEED;
    b.vy = -0.5f;

    p1.score = p2.score = 0;
    p1.pos.w = p2.pos.w = board_width;
    p1.pos.h = p2.pos.h = 150;
    p1.speed = 7.5;
    p2.speed = 7.5;

    p1.pos.x = board_width / 2 + 10;
    p2.pos.x = SCREEN_WIDTH - p2.pos.w - 10 - p2.pos.w / 2;

    p1.pos.y = SCREEN_HEIGHT / 2 - p1.pos.h / 2;
    p2.pos.y = SCREEN_HEIGHT / 2 - p2.pos.h / 2;

    printf("Starting Game Loop\n");

    Uint32 prevTime = 0;
    bool quit = false, annot = false;
    int frames = 0;
    float fps = -1.f;
    char buffer[128];
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    srand((int)time(NULL));

    // FPS Calcule
    Uint32 start = SDL_GetTicks();

    while (!quit) {


        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)  quit = true;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    quit = true;
                    break;
                }
            }
        }


           KeyPress = 0;
        // Player Movement
        if (keystates[SDL_SCANCODE_W]) {
            p1.pos.y -= p1.speed;
            KeyPress = SDL_SCANCODE_W;
        }
        if (keystates[SDL_SCANCODE_S]) {
            p1.pos.y += p1.speed;
            KeyPress = SDL_SCANCODE_S;
        }


        if (KeyPrMess == SDL_SCANCODE_UP)
            p2.pos.y -= p2.speed;
        if (KeyPrMess == SDL_SCANCODE_DOWN)
            p2.pos.y += p2.speed;

        if (b.vx > BALL_MAXSPEED)
            b.vx = BALL_MAXSPEED;

        if (b.vy > BALL_MAXSPEED)
            b.vy = BALL_MAXSPEED;

        // Update Ball coord
        if (!annot) {
            if (menuState <=0)
            {
                b.x += b.vx;
                b.y += b.vy;
            }
        }
        else
        {
            if (prevTime == 0)
                prevTime = SDL_GetTicks();

            Uint32 currTime = SDL_GetTicks();
            float secondsElapsed = (currTime - prevTime) / 1000.0f;
            if (secondsElapsed > 1.99) {
                prevTime = 0;
                annot = false;
            }


        }

        if (b.y < 0) {
            b.y = 0;
            b.vy *= -1;
        }
        if (b.y + BALL_HEIGHT >= SCREEN_HEIGHT) {
            b.y = SCREEN_HEIGHT - BALL_HEIGHT - 1;
            b.vy *= -1;
        }

        if (b.x < 0) {


            p2.score += 1;
            b.x = p1.pos.x + p1.pos.w;
            b.y = p1.pos.y + p1.pos.h / 2;
            b.vx = BALL_INIT_SPEED;
            b.speed = BALL_INIT_SPEED;
            annot = true;
        }
        if (b.x + BALL_WIDTH >= SCREEN_WIDTH) {


            p1.score += 1;
            b.x = p2.pos.x - BALL_WIDTH;
            b.y = p2.pos.y + p2.pos.h / 2;
            b.vx = -1 * BALL_INIT_SPEED;
            b.speed = BALL_INIT_SPEED;
            annot = true;
        }

        if (p1.pos.y < 0) p1.pos.y = 0;
        if (p1.pos.y + p1.pos.h > SCREEN_HEIGHT) p1.pos.y = SCREEN_HEIGHT - p1.pos.h;
        if (p2.pos.y < 0) p2.pos.y = 0;
        if (p2.pos.y + p2.pos.h > SCREEN_HEIGHT) p2.pos.y = SCREEN_HEIGHT - p2.pos.h;

            b_rect.x = (int)b.x;
            b_rect.y = (int)b.y;


        // Player Collision
        if (SDL_HasIntersection(&p1.pos, &b_rect)) {

            Mix_PlayChannel(-1, collisionSound, 0);

            b.x = p1.pos.x + p1.pos.w;

            b.speed = b.speed * BALL_ACCELERATE;

            float angle = calc_angle(p1.pos.y, b.y, p1.pos.h);
            b.vx = b.speed * cos(angle);
            b.vy = ((b.vy > 0) ? -1 : 1) * b.speed * sin(angle);
        }
        if (SDL_HasIntersection(&p2.pos, &b_rect)) {

            Mix_PlayChannel(-1, collisionSound, 0);

            b.x = p2.pos.x - BALL_WIDTH;

            b.speed = b.speed * BALL_ACCELERATE;

            float angle = calc_angle(p2.pos.y, b.y, p2.pos.h);
            b.vx = -1 * b.speed * cos(angle);
            b.vy = ((b.vy > 0) ? -1 : 1) * b.speed * sin(angle);
        }

        SDL_RenderClear(ren);

        SDL_RenderCopy(ren, squareTex, NULL, &p1.pos);
        SDL_RenderCopy(ren, squareTex, NULL, &p2.pos);

        renderTexture(squareTex, ren, SCREEN_WIDTH / 2 - CENTER_WIDTH / 2, 0, 2, SCREEN_HEIGHT);

        // Draw the Ball
        renderTexture(squareTex, ren, b.x, b.y, BALL_WIDTH, BALL_HEIGHT);

        // Display the score
        sprintf(buffer, "%d", p1.score);
        SDL_Texture* p1score = renderText(buffer, "fonts/typomoderno.ttf", whiteColor, 30, ren);
        sprintf(buffer, "%d", p2.score);
        SDL_Texture* p2score = renderText(buffer, "fonts/typomoderno.ttf", whiteColor, 30, ren);

        int width;
        SDL_QueryTexture(p1score, NULL, NULL, &width, NULL);

        renderTexture(p1score, ren, SCREEN_WIDTH / 2 - width - 10, 10, -1, -1);
        renderTexture(p2score, ren, SCREEN_WIDTH / 2 + 10, 10, -1, -1);

        SDL_DestroyTexture(p1score);
        SDL_DestroyTexture(p2score);

        if (menuState > 0)
        {
            SDL_Color SColor = {255, 127, 39};

            sprintf(buffer, "%u", menuState);
            SDL_Texture *StartGame = renderText(buffer, "fonts/typomoderno.ttf", SColor, 90, ren);

            int width;
            SDL_QueryTexture(StartGame, NULL, NULL, &width, NULL);

            renderTexture(StartGame, ren, SCREEN_WIDTH / 2 - width - 10, SCREEN_HEIGHT / 2, -1, -1);
            SDL_DestroyTexture(StartGame);
        }


        Uint32 end = SDL_GetTicks();

        float secondsElapsed = (end - start) / 1000.0f;

        if (fps < 0.f) {
            fps = 1.f / secondsElapsed; frames = 0;
        }

        frames += 1;
        if (secondsElapsed > 0.999)
        {
            fps = frames;
            frames = 0;
            start = end;
        }

        sprintf(buffer, "Pong 2 - Server    FPS: %.0f", fps);
        SDL_SetWindowTitle(win, buffer);
        SDL_RenderPresent(ren);

        if(newsockfd < 0) {
          newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
            error("ERROR on accept");
        }

         bzero(Messag,256);
         n = read(newsockfd,Messag,255);
         if (n < 0) error("ERROR reading from socket");
         Uint8 Ms;
		 sscanf(Messag, "%u %u", &KeyPrMess, &Ms);
		 menuState = Ms;

		 bzero(Messag,256);
		 sprintf(Messag, "%f %f %i %i %u", b.x, b.y, p1.score, p2.score, KeyPress);
		 n = write(newsockfd,Messag,strlen(Messag));
		 if (n < 0) error("ERROR writing to socket");

	}

    close(newsockfd);
    close(sockfd);
    SDL_DestroyTexture(squareTex);
    Cleanup(&ren, &win);
    return 0;
}

void Initialise(SDL_Renderer** ren, SDL_Window** win) {
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
        sdl_bomb("Failed to Initialise SDL");

    *win = SDL_CreateWindow(
        "This will surely be overwritten",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (*win == NULL)
        sdl_bomb("Failed to create SDL Window");

    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (*ren == NULL)
        sdl_bomb("Failed to create SDL Renderer");

    const int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (IMG_Init(flags) != flags)
        sdl_bomb("Failed to load the Image loading extensions");

    if (TTF_Init() != 0)
        sdl_bomb("Failed to load TTF extension");


    //Initialize SDL_mixer
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
        sdl_bomb("Failed to load MIX extension");

}

void Cleanup(SDL_Renderer** ren, SDL_Window** win) {
    SDL_DestroyRenderer(*ren);
    SDL_DestroyWindow(*win);
    Mix_FreeChunk(collisionSound);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
