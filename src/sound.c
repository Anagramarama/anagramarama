/*
Anagramarama - A word game.  Like anagrams?  You'll love anagramarama!
Copyright (C) 2003  Colm Gallagher

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Contact Details: colm@coralquest.com
		 12 Weston Terrace, West Kilbride, KA23 9JX.  Scotland.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <pthread.h>

struct sound{
	char* name;
	SDL_AudioSpec spec;
	Uint8 *audio_chunk;
	Uint32 audio_len;
	Uint8 *audio_pos;
	struct sound* next;
};

struct soundQueue{
	char* name;
	struct soundQueue* next;
};

//Uint32 audio_len;
//Uint8 *audio_pos;

//struct sound* soundCache = NULL;
//struct soundQueue* sounds = NULL;

/*******************************************************************************/
//void fill_audio(void *udata, Uint8 *stream, int len){
// The audio function callback takes the following parameters:
//   stream:  A pointer to the audio buffer to be filled
//   len:     The length (in bytes) of the audio buffer

	/* Only play if we have data left */
//	if ( audio_len == 0 )
//		return;

	/* Mix as much data as possible */
//	len = ( len > audio_len ? audio_len : len );
//	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
//	audio_pos += len;
//	audio_len -= len;
//}

/*******************************************************************************/
void pushSound(struct sound** soundCache, char* name, char* filename){

struct sound* thisSound = NULL;

	thisSound = malloc(sizeof(struct sound));
	thisSound->name = malloc(sizeof(name)*strlen(name));
	strcpy(thisSound->name, name);
	/* Set the audio format */
	thisSound->spec.freq = 22050;
	thisSound->spec.format = AUDIO_S16;
	thisSound->spec.channels = 2;    /* 1 = mono, 2 = stereo */
	thisSound->spec.samples = 1024;  /* Good low-latency value for callback */
	thisSound->spec.callback = NULL; // null for now - we'll set the local ref in the play routine
	thisSound->spec.userdata = NULL;
	thisSound->next = *soundCache;

	if (!strcmp(name, "playArea")){
		// create a big area to mix and play the audio in
//		thisSound->audio_len = malloc(sizeof(Uint32));
		thisSound->audio_len = 8152;
		thisSound->audio_chunk = malloc(sizeof(Uint8)*thisSound->audio_len);
	}
	else{
		// Attempt to load a sample
		if (!SDL_LoadWAV(filename, &thisSound->spec, &thisSound->audio_chunk, &thisSound->audio_len)){
			SDL_CloseAudio();
			fprintf(stderr, "Error: %s", SDL_GetError());
			return;
		}
	}

	*soundCache = thisSound;
}

/*******************************************************************************/
void bufferSounds(struct sound** soundCache){

	pushSound(&(*soundCache),"click", "audio/click.wav");
	pushSound(&(*soundCache),"click2", "audio/click2.wav");
	pushSound(&(*soundCache),"found", "audio/found.wav");
	pushSound(&(*soundCache),"playArea", "");
}

/*******************************************************************************/
void clearSoundBuffer(struct sound** soundCache){

struct sound* currentSound = *soundCache, *previousSound = NULL;

	printf("top\n");

	while (currentSound!=NULL){

		printf("in top\n");

		SDL_FreeWAV(currentSound->audio_chunk);
		free(currentSound->name);
		previousSound = currentSound;
		currentSound = currentSound->next;
		free(previousSound);

		printf("in botton\n");
	}

	printf("bottom\n");
}

/*******************************************************************************/
//void playSound(struct sound* soundCache, char* name){

//struct sound* currentSound = soundCache;

//	while (currentSound!=NULL){
//		if (!strcmp(currentSound->name, name)){
//			printf("current sound is : %s\n", currentSound->name);
//			audio_pos = currentSound->audio_chunk;
//			audio_len = currentSound->audio_len;
			/* Open the audio device, forcing the desired format */
//			currentSound->spec.callback = fill_audio;
//			if ( SDL_OpenAudio(&currentSound->spec, NULL) < 0 ) {
//				fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
//				return;
//			}

			// Unpause the audio
//				SDL_PauseAudio(0);

			/* Wait for sound to complete */
//			while ( audio_len > 0 ) {
//				SDL_Delay(100);         /* Sleep 1/10 second */
//			}

//			SDL_CloseAudio();
//			break;

//		}
//		currentSound = currentSound->next;
//	}
//}

/*******************************************************************************/
void pushSoundQueue(struct soundQueue** sounds, char* name){

struct soundQueue* current = *sounds;
struct soundQueue* previous = NULL;


	// get the tail - won't ever be very many, so not too lossy
	// and saves handling a tail pointer
	while (current != NULL){
		previous = current;
		current = current->next;
	}
	
	current = NULL;
	// OK a bit nasty - i'm reusing current here, i only
	// used it above for running through the list

	current = malloc(sizeof(struct soundQueue));
	current->name = malloc(sizeof(char)*strlen(name));
	strcpy(current->name,name);
	current->next = NULL;


	// join up the queue
	if (previous == NULL)
		// first time only, the head is previous
		*sounds = current;
	else
		previous->next = current;

}

/*******************************************************************************/
void popSoundQueue(struct soundQueue** sounds, char* name){

struct soundQueue* oldHead = *sounds;

	*sounds = oldHead->next;

	free(oldHead->name);
	free(oldHead);
}

/*******************************************************************************/
//int doneSounds = 0;
//int audioThreadOpen = 0;
//void* handleSounds(void *arg){

//char* name;

//	audioThreadOpen = 1;

	// if we're currently playing a sound or have none to handle - just leave
//	while(!doneSounds){

//		if(SDL_GetAudioStatus() == SDL_AUDIO_STOPPED && sounds != NULL){
			// get next sound off the queue and then pop it
//			name = malloc(sizeof(char)*strlen(sounds->name));
//			strcpy(name, sounds->name);
//			popSoundQueue(&sounds, name);
//			printf("sound: %s\n", name);
//			playSound(soundCache, name);

//			free(name);
//		}
//	}
//	audioThreadOpen = 0;
//	return 0;
//}

/*******************************************************************************/
/*int main(int argc, char** argv){

SDL_Surface* screen = NULL;
SDL_Event event;
Uint8* key;

pthread_t audio;

	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0){
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	screen=SDL_SetVideoMode(640,480,8,0);

	atexit(SDL_Quit);

	soundCache=NULL;
	soundCache = malloc(sizeof(struct sound));
	bufferSounds(&soundCache);

	//sounds = malloc(sizeof(struct soundQueue));
	pushSoundQueue(&sounds, "click");
	pushSoundQueue(&sounds, "click2");
	pushSoundQueue(&sounds, "click");
	pushSoundQueue(&sounds, "click2");
	pushSoundQueue(&sounds, "click2");
	pushSoundQueue(&sounds, "found");

	// create the audio handler on a new thread
	pthread_create(&audio, NULL, handleSounds, NULL);

//	playSound(soundCache, "click");
//	playSound(soundCache, "click2");
//	playSound(soundCache, "found");

	do{
		key = SDL_GetKeyState(NULL);
		if(SDL_PollEvent(&event))
				if(event.type == SDL_QUIT)
					break;
	}
	while (!key[SDLK_ESCAPE]);

	doneSounds=1;
	while (audioThreadOpen){
		SDL_Delay(500);
	}
	
	clearSoundBuffer(&soundCache);
	return 0;
}*/
