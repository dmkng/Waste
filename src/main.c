#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

SDL_Texture* RenderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color);
void DisplayError(const char* title, uint8_t type, const char* desc);

int main(int argc, char* argv[]) {
	// Window title
	const char* title = "Waste";

	// Window dimensions
	int width = 800;
	int height = 600;

	// For resizing and setting position
	SDL_Rect upper_rect;
	SDL_Rect lower_rect;
	SDL_Rect rect;

	// For events
	SDL_Event e;

	// For timers
	uint32_t counter_ticks;
	uint32_t frame_ticks;

	// Frame ticks
	uint32_t ticks;

	// Static FPS value
	uint32_t fps = 60;

	// Counter
	uint64_t count = 0;

	// Is muted
	uint8_t muted = 0;

	// Arguments loop
	for(int i = 1; i < argc; i++) {
		if(!muted && (!strcmp(argv[i], "--mute") || !strcmp(argv[i], "-m"))) {
			muted = 1;
		}
	}

	// Init SDL
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		DisplayError(title, 0, "Can't init SDL");
		return 1;
	}

	// Create window
	SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	if(window == NULL) {
		DisplayError(title, 0, "Can't create window");
		SDL_Quit();
		return 1;
	}

	// Create renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == NULL) {
		DisplayError(title, 0, "Can't create renderer");
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Set window background color
	if(SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) != 0) {
		DisplayError(title, 0, "Can't create window");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Init font engine
	if(TTF_Init() < 0) {
		DisplayError(title, 1, "Can't init font engine");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Load fonts
	TTF_Font* normal_font = TTF_OpenFont("res/DroidSans.ttf", 30);
	TTF_Font* counter_font = TTF_OpenFont("res/DroidSans.ttf", 80);
	if(normal_font == NULL || counter_font == NULL) {
		DisplayError(title, 1, "Can't load required fonts");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	// Open audio device
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
		DisplayError(title, 2, "Can't open audio device");
		TTF_CloseFont(normal_font);
		TTF_CloseFont(counter_font);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		Mix_Quit();
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	// Load sounds
	Mix_Music* music = Mix_LoadMUS("res/music.wav");
	if(music == NULL) {
		DisplayError(title, 2, "Can't load required sounds");
		TTF_CloseFont(normal_font);
		TTF_CloseFont(counter_font);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		Mix_CloseAudio();
		Mix_Quit();
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	// Colors
	SDL_Color black = { 0, 0, 0, 255 };

	// Create upper text
	SDL_Texture* upper_text = RenderText(renderer, normal_font, "You just wasted", black);

	// Get upper text size
	SDL_QueryTexture(upper_text, NULL, NULL, &upper_rect.w, &upper_rect.h);

	// Set upper text position
	upper_rect.x = width / 2 - upper_rect.w / 2;
	upper_rect.y = 210;

	// Create lower text
	SDL_Texture* lower_text = RenderText(renderer, normal_font, "seconds of your life.", black);

	// Get lower text size
	SDL_QueryTexture(lower_text, NULL, NULL, &lower_rect.w, &lower_rect.h);

	// Set lower text position
	lower_rect.x = width / 2 - lower_rect.w / 2;
	lower_rect.y = height - 210 - lower_rect.h;

	// Create counter text
	SDL_Texture* counter = RenderText(renderer, counter_font, "0", black);

	// Play music
	Mix_PlayMusic(music, -1);

	// Pause music if it's muted
	if(muted) {
		Mix_PauseMusic();
	}

	// Start counter timer
	counter_ticks = SDL_GetTicks();

	while(1) {
		// Events
		if(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				break;
			} else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m) { // key M pressed down
				muted = !muted;
				if(muted) {
					Mix_PauseMusic();
				} else {
					Mix_ResumeMusic();
				}
			}
		}

		// Counter
		ticks = SDL_GetTicks() - counter_ticks;
		if(ticks >= 1000) { // 1 second elapsed
			// Restart counter timer
			counter_ticks = SDL_GetTicks();

			// Increment counter and convert it to string
			count += ticks / 1000;
			char text[21];
			snprintf(text, 20, "%" PRIu64, count);

			// Refresh counter text
			SDL_DestroyTexture(counter);
			counter = RenderText(renderer, counter_font, text, black);
		}

		// Start frame timer
		frame_ticks = SDL_GetTicks();

		// Clear renderer
		SDL_RenderClear(renderer);

		// Display upper text
		SDL_RenderCopy(renderer, upper_text, NULL, &upper_rect);

		// Display lower text
		SDL_RenderCopy(renderer, lower_text, NULL, &lower_rect);

		// Get counter text size
		SDL_QueryTexture(counter, NULL, NULL, &rect.w, &rect.h);

		// Set counter text position
		rect.x = width / 2 - rect.w / 2;
		rect.y = height / 2 - rect.h / 2;

		// Display counter text
		SDL_RenderCopy(renderer, counter, NULL, &rect);

		// Show render
		SDL_RenderPresent(renderer);

		// Wait remaining time
		ticks = SDL_GetTicks() - frame_ticks;
		if(ticks < 1000 / fps) {
			SDL_Delay(1000 / fps - ticks);
		}
	}

	// Destroy textures
	SDL_DestroyTexture(upper_text);
	SDL_DestroyTexture(lower_text);
	SDL_DestroyTexture(counter);

	// Destroy sounds
	Mix_FreeMusic(music);

	// Destroy fonts
	TTF_CloseFont(normal_font);
	TTF_CloseFont(counter_font);

	// Destroy renderer and window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	// Close audio device
	Mix_CloseAudio();

	// Quit SDL
	TTF_Quit();
	SDL_Quit();

	return 0;
}

////////////////////////////////////////////////// Functions //////////////////////////////////////////////////

void DisplayError(const char* title, uint8_t type, const char* desc) {
	// Get SDL Error
	const char* err;
	switch(type) {
		case 1:
			err = TTF_GetError();
			break;
		case 2:
			err = Mix_GetError();
			break;
		default:
			err = SDL_GetError();
	}

	// Calculate string lengths
	size_t desc_len = strlen(desc);
	size_t err_len = strlen(err);

	// Build string
	char* str = malloc(desc_len + err_len + 4);
	memcpy(str, desc, desc_len);
	memcpy(str + desc_len, " (", 2);
	memcpy(str + desc_len + 2, err, err_len);
	memcpy(str + desc_len + 2 + err_len, ")\0", 2);

	// Display string
	printf("%s\n", str);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, str, NULL);
}

SDL_Texture* RenderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color) {
	// Render surface with text
	SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
	if(surface == NULL) {
		return NULL;
	}

	// Convert surface to texture
	SDL_Texture* rendered_text = SDL_CreateTextureFromSurface(renderer, surface);

	// Destroy surface
	SDL_FreeSurface(surface);

	return rendered_text;
}
