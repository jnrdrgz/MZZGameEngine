#pragma once

#include "SoundManager.h"

std::unordered_map<std::string, Mix_Chunk*> SoundManager::chunks;

SoundManager::SoundManager() {
	//chuncks.reserve(10);


	//chuncks.push_back(m);
}

void SoundManager::load(std::string name, std::string path) {
	auto load_audio = [](std::string file_path) {
		Mix_Chunk* m = Mix_LoadWAV(file_path.c_str());
		if (!m) std::cout << "error loading audio " << file_path << "\n";
		return m;
	};

	chunks[name] = load_audio(path);
}

SoundManager::~SoundManager() {
	for (auto c : chunks) {
		Mix_FreeChunk(c.second);
	}
}

void SoundManager::play_chunk(std::string sound) {
	Mix_PlayChannel(-1, chunks[sound], 0);
}

int SoundManager::play_loop_chunk(std::string sound) {
	return Mix_PlayChannel(-1, chunks[sound], -1);
}

void SoundManager::shut_up_channel(int channel) {
	Mix_HaltChannel(channel);
}

int SoundManager::play_for_seconds(std::string sound, int ms) {
	return Mix_PlayChannelTimed(-1, chunks[sound], -1, ms);
}

void SoundManager::play_music(std::string sound) {

}

//std::vector<> chuncks;
