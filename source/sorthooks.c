void InitSortHooks()
{
    uint64_t base_address = get_base_address();

    SortByArtist = (void*)(base_address + 0x00ca5e30);
    SortByTitle = (void*)(base_address + 0x00ca5d60);
    SortByStars = (void*)(base_address + 0x00ca5f8c); // supposed to be 0x00ca5f80

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
