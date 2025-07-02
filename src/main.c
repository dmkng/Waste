// Main handling is not needed at least on Windows and Linux
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// Window title
#define TITLE "Waste"

// Window dimensions
#define WIDTH 800
#define HEIGHT 600

// Text lines
#define UPPER "You just wasted"
#define LOWER "seconds of your life."

// FPS lock in addition to VSync
//#define FPS 60

struct Data {
	SDL_Renderer *renderer;

	// For counter rendering
	TTF_Font *counter_font;

	// Cached text lines
	SDL_Texture *upper_text, *lower_text, *counter_text;

	// For resizing and positioning text lines
	SDL_Rect upper_rect, lower_rect, counter_rect;

	// Counter text
	char text[21];

	// Counter timer
	uint32_t counter_ticks;

	// Counter
	uint64_t count;
} data;

void Update(struct Data *data);
void DisplayError(uint8_t type, const char *desc, size_t desclen);
SDL_Texture *RenderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color);

// Error description macro
#define DESC(str) str, sizeof(str)

// For updating the window when it's being moved
static int SDLCALL ExposeEventWatcher(void *userdata, SDL_Event *e) {
	if(e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_EXPOSED) {
		Update((struct Data *)userdata);
	}
	return 0;
}

int main(int argc, char *argv[]) {
	// For events
	SDL_Event e;

	#ifdef FPS
		// FPS lock timer
		uint32_t ticks;
	#endif

	// Is muted
	uint8_t muted = 0;

	// Arguments loop
	for(int i = 1; i < argc; ++i) {
		if(!muted && (!strcmp(argv[i], "--mute") || !strcmp(argv[i], "-m"))) {
			muted = 1;
		}
	}

	// Init SDL
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		DisplayError(0, DESC("Can't init SDL"));
		return 1;
	}

	// Init font engine
	if(TTF_Init() < 0) {
		DisplayError(1, DESC("Can't init font engine"));
		SDL_Quit();
		return 1;
	}

	// Create window
	SDL_Window *window = SDL_CreateWindow(
		TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0
	);
	if(window == NULL) {
		DisplayError(0, DESC("Can't create window"));
		SDL_Quit();
		return 1;
	}

	// Create renderer
	data.renderer = SDL_CreateRenderer(
		window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);
	if(data.renderer == NULL) {
		DisplayError(0, DESC("Can't create renderer"));
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Set window background color
	if(SDL_SetRenderDrawColor(data.renderer, 255, 255, 255, 255) != 0) {
		DisplayError(0, DESC("Can't init window"));
		SDL_DestroyRenderer(data.renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Load fonts
	TTF_Font *normal_font = TTF_OpenFont("res/DroidSans.ttf", 30);
	if(normal_font == NULL) {
		DisplayError(1, DESC("Can't load required fonts"));
		SDL_DestroyRenderer(data.renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return 1;
	}
	data.counter_font = TTF_OpenFont("res/DroidSans.ttf", 80);
	if(data.counter_font == NULL) {
		DisplayError(1, DESC("Can't load required fonts"));
		TTF_CloseFont(normal_font);
		SDL_DestroyRenderer(data.renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	// Open audio device
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
		DisplayError(2, DESC("Can't open audio device"));
		TTF_CloseFont(normal_font);
		TTF_CloseFont(data.counter_font);
		SDL_DestroyRenderer(data.renderer);
		SDL_DestroyWindow(window);
		Mix_Quit();
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	// Load sounds
	Mix_Music *music = Mix_LoadMUS("res/music.wav");
	if(music == NULL) {
		DisplayError(2, DESC("Can't load required sounds"));
		TTF_CloseFont(normal_font);
		TTF_CloseFont(data.counter_font);
		SDL_DestroyRenderer(data.renderer);
		SDL_DestroyWindow(window);
		Mix_CloseAudio();
		Mix_Quit();
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	// Create counter text
	data.counter_text = RenderText(data.renderer, data.counter_font, "0", (SDL_Color){ 0, 0, 0, 255 });

	// Get counter text size
	SDL_QueryTexture(data.counter_text, NULL, NULL, &data.counter_rect.w, &data.counter_rect.h);

	// Create upper text
	data.upper_text = RenderText(data.renderer, normal_font, UPPER, (SDL_Color){ 0, 0, 0, 255 });

	// Get upper text size
	SDL_QueryTexture(data.upper_text, NULL, NULL, &data.upper_rect.w, &data.upper_rect.h);

	// Set upper text position
	data.upper_rect.x = WIDTH / 2 - data.upper_rect.w / 2;
	data.upper_rect.y = HEIGHT / 2 - data.counter_rect.h / 2 - 10 - data.upper_rect.h;

	// Create lower text
	data.lower_text = RenderText(data.renderer, normal_font, LOWER, (SDL_Color){ 0, 0, 0, 255 });

	// Get lower text size
	SDL_QueryTexture(data.lower_text, NULL, NULL, &data.lower_rect.w, &data.lower_rect.h);

	// Set lower text position
	data.lower_rect.x = WIDTH / 2 - data.lower_rect.w / 2;
	data.lower_rect.y = HEIGHT / 2 + data.counter_rect.h / 2 + 10;

	// Play music
	Mix_PlayMusic(music, -1);

	// Pause music if it's muted
	if(muted) {
		Mix_PauseMusic();
	}

	// Start counter
	data.counter_ticks = SDL_GetTicks();
	data.count = 0;

	// Update the window when it's being moved
	SDL_AddEventWatch(ExposeEventWatcher, &data);

	while(1) {
		// Events
		if(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				break;
			} else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m) {
				// Key M pressed down
				muted = !muted;
				if(muted) {
					Mix_PauseMusic();
				} else {
					Mix_ResumeMusic();
				}
			}
		}

		#ifdef FPS
			// Start frame timer
			ticks = SDL_GetTicks();
		#endif

		Update(&data);

		#ifdef FPS
			// Wait remaining time
			ticks = SDL_GetTicks() - ticks;
			if(ticks < 1000 / FPS) {
				SDL_Delay(1000 / FPS - ticks);
			}
		#endif
	}

	// Destroy textures
	SDL_DestroyTexture(data.upper_text);
	SDL_DestroyTexture(data.lower_text);
	SDL_DestroyTexture(data.counter_text);

	// Destroy sounds
	Mix_FreeMusic(music);

	// Destroy fonts
	TTF_CloseFont(normal_font);
	TTF_CloseFont(data.counter_font);

	// Destroy renderer and window
	SDL_DestroyRenderer(data.renderer);
	SDL_DestroyWindow(window);

	// Close audio device
	Mix_CloseAudio();

	// Quit SDL
	TTF_Quit();
	SDL_Quit();

	return 0;
}

//////////////////////////////////////// Functions ////////////////////////////////////////

void Update(struct Data *data) {
	// Counter
	uint32_t ticks = SDL_GetTicks() - data->counter_ticks;
	if(ticks >= 1000) { // 1 second elapsed
		// Restart counter timer
		data->counter_ticks = SDL_GetTicks();

		// Increment counter and convert it to string
		data->count += ticks / 1000;
		snprintf(data->text, 21, "%" PRIu64, data->count);

		// Refresh counter text
		SDL_DestroyTexture(data->counter_text);
		data->counter_text = RenderText(data->renderer, data->counter_font, data->text, (SDL_Color){ 0, 0, 0, 255 });
	}

	// Clear renderer
	SDL_RenderClear(data->renderer);

	// Display upper text
	SDL_RenderCopy(data->renderer, data->upper_text, NULL, &data->upper_rect);

	// Display lower text
	SDL_RenderCopy(data->renderer, data->lower_text, NULL, &data->lower_rect);

	// Get counter text size
	SDL_QueryTexture(data->counter_text, NULL, NULL, &data->counter_rect.w, &data->counter_rect.h);

	// Set counter text position
	data->counter_rect.x = WIDTH / 2 - data->counter_rect.w / 2;
	data->counter_rect.y = HEIGHT / 2 - data->counter_rect.h / 2;

	// Display counter text
	SDL_RenderCopy(data->renderer, data->counter_text, NULL, &data->counter_rect);

	// Show render
	SDL_RenderPresent(data->renderer);
}


void DisplayError(uint8_t type, const char *desc, size_t desc_len) {
	// Get SDL Error
	const char *err;
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

	// Calculate string length
	size_t err_len = strlen(err);

	// Build string (very fast way)
	char *str = malloc(desc_len + err_len + 3);
	memcpy(str, desc, desc_len - 1);
	memcpy(str + desc_len - 1, " (", 2);
	memcpy(str + desc_len + 1, err, err_len);
	memcpy(str + desc_len + err_len + 1, ")\0", 2);

	// Display string
	puts(str);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, str, NULL);
	free(str);
}

SDL_Texture *RenderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color) {
	// Render surface with text
	SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
	if(surface == NULL) {
		return NULL;
	}

	// Convert surface to texture
	SDL_Texture *rendered_text = SDL_CreateTextureFromSurface(renderer, surface);

	// Destroy surface
	SDL_FreeSurface(surface);

	return rendered_text;
}
