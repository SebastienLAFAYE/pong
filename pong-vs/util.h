#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

void logSDLError(const char *msg) 
{
    printf("error: %s\n", SDL_GetError());
}

void sdl_bomb(const char *msg) 
{
    logSDLError(msg);
    exit(-1);
}

SDL_Texture *renderText(const char *msg, const char *fontPath, SDL_Color color, int fontSize, SDL_Renderer *ren) 
{
    TTF_Font *font = TTF_OpenFont(fontPath, fontSize);
    if(font == NULL) 
    {
        logSDLError("Unable to open font");
        return NULL;
    }

    SDL_Surface *surface = TTF_RenderText_Blended(font, msg, color);
    if(surface == NULL) 
    {
        TTF_CloseFont(font);
        logSDLError("Unable to render text to a surface");
        return NULL;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surface);
    if(tex == NULL) 
    {
        logSDLError("Unable to render surface to texture");
    }

    SDL_FreeSurface(surface);
    TTF_CloseFont(font);

    return tex;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h) 
{
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;

    // If no width and height are specified, use the texture's actual width and height
    if(w == -1 || h == -1)
        SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);

    SDL_RenderCopy(ren, tex, NULL, &dest);
}
