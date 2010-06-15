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

//void fill_audio(void *udata, Uint8 *stream, int len);
void pushSound(struct sound** soundCache, char* name, char* filename);
void bufferSounds(struct sound** soundCache);
void clearSoundBuffer();
void playSound(struct sound* soundCache, char* name);
void pushSoundQueue(struct soundQueue** sounds, char* name);
void popSoundQueue(struct soundQueue** sounds, char* name);
void* handleSounds(void *arg);
