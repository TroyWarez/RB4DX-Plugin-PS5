#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include "plugin_common.h"
#include "sorthooks.h"

// Hook definitions restored from the PS4 RB4DX-Plugin sorthooks.c

bool sortingbyartist = false;

int(*SortByArtist)(void*, void*);
HOOK_INIT(SortByArtist);
int SortByArtist_hook(void* song1, void* song2) {
    sortingbyartist = true;
    //final_printf("SortByArtist\n");
    return HOOK_CONTINUE(SortByArtist, int (*)(void*, void*), song1, song2);
}

int(*SortByTitle)(void*, void*);
HOOK_INIT(SortByTitle);
int SortByTitle_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByTitle\n");
    return HOOK_CONTINUE(SortByTitle, int (*)(void*, void*), song1, song2);
}

// for some reason this never gets hit, while the rest of the sort functions that arent title/artist crash
int(*SortByStars)(void*, void*);
HOOK_INIT(SortByStars);
int SortByStars_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByStars\n");
    return HOOK_CONTINUE(SortByStars, int (*)(void*, void*), song1, song2);
}

// PS5 version of InitSortHooks (16 KB page-aligned mprotect)
void InitSortHooks()
{
    uint64_t base_address = get_base_address();

    SortByArtist = (void*)(base_address + 0x00ca5e30);
    SortByTitle = (void*)(base_address + 0x00ca5d60);
    SortByStars = (void*)(base_address + 0x00ca5f8c); // supposed to be 0x00ca5f80

    // temporary sort by stars patch to fix the sorting by artist check
    char SortByStarsPatch[] = { 0xe9, 0x1f, 0x01, 0x00, 0x00 };
    uintptr_t patch_addr = (uintptr_t)SortByStars;
    uintptr_t page_start = patch_addr & ~0x3FFFULL;
    if (sceKernelMprotect((void*)page_start, 0x4000, VM_PROT_ALL) < 0) {
        sceKernelMprotect((void*)(patch_addr & ~0xFFFULL), 0x1000, VM_PROT_ALL);
    }
    memcpy(SortByStars, SortByStarsPatch, sizeof(SortByStarsPatch));

    HOOK(SortByArtist);
    HOOK(SortByTitle);
}

void DestroySortHooks()
{
    UNHOOK(SortByArtist);
    UNHOOK(SortByTitle);
}
