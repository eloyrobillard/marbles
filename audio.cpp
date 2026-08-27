#include "audio.hpp"

SDL_AudioSpec AudioMachine::backtrackSpec;
u8 *AudioMachine::backtrackBuffer;
u32 AudioMachine::backtrackLength;
SDL_AudioStream *AudioMachine::backtrackStream;

SDL_AudioSpec AudioMachine::oneOffSpec;
u8 *AudioMachine::oneOffBuffer;
u32 AudioMachine::oneOffLength;
SDL_AudioStream *AudioMachine::oneOffStream;

SDL_AudioSpec AudioMachine::rollSpec;
u8 *AudioMachine::rollBuffer;
u32 AudioMachine::rollLength;
SDL_AudioStream *AudioMachine::rollStream;
