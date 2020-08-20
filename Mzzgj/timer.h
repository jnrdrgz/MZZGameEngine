#pragma once

#include <SDL.h>

class Timer
{
public:
    Timer() {
        this->startTime = 0;
        this->pausedTime = 0;
        this->started = false;
        this->paused = false;
    }

    void start() {
        started = true;
        paused = false;
        startTime = SDL_GetTicks();
        pausedTime = 0;
    }

    void stop() {
        started = false;
        paused = false;
        startTime = 0;
        pausedTime = 0;
    }

    void pause() {
        if (started && !paused) {
            started = false;
            paused = true;
            startTime = 0;
            pausedTime = SDL_GetTicks() - startTime;
        }
    }

    void unpause() {
        if (started && paused) {
            paused = false;
            startTime = SDL_GetTicks() - pausedTime;
            pausedTime = 0;
        }
    }

    Uint32 getTicks() {
        Uint32 time = 0;

        if (started) {
            if (paused) {
                time = pausedTime;
            }
            else {
                time = SDL_GetTicks() - startTime;
            }

        }

        return time;
    }

    Uint32 getTicks_in_seconds() {
        return getTicks() / 1000;
    }

    bool isStarted() { return started; }
    bool isPaused() { return paused && started; }

private:
    Uint32 startTime;
    Uint32 pausedTime;
    bool paused;
    bool started;
};