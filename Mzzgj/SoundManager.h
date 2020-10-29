#pragma once
#include <SDL_mixer.h>
#include <string>
#include <iostream>
#include <unordered_map>

struct SoundManager
{
	SoundManager();

	void load(std::string name, std::string path);

	~SoundManager();

	static void play_chunk(std::string sound);

	static int play_loop_chunk(std::string sound);

	static void shut_up_channel(int channel);

	static int play_for_seconds(std::string sound, int ms);

	static void play_music(std::string sound);

	//std::vector<> chuncks;

	static std::unordered_map<std::string, Mix_Chunk*> chunks;
};

