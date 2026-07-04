#include "music.h"
#include <stdio.h>

// Wrap the logic inside a proper function!
void StartStageMusic() {
    
    // 1. Load the file path into a Mix_Music pointer first
    Mix_Music* StageMusic = Mix_LoadMUS("assets/audio/Test_Zone.mid");
    
    // 2. Always error check to make sure the path isn't broken
    if (!StageMusic) {
        fprintf(stderr, "Failed to load music file: %s\n", Mix_GetError());
        return;
    }

    // 3. NOW you can pass the pointer into your PlayMusic function safely!
    PlayMusic(StageMusic, 1.0f, 1.0f);
}