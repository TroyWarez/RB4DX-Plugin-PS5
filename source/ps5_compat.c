// source/ps5_compat.c
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <orbis/libkernel.h>
#include <GoldHEN.h>

typedef struct {
    size_t size;
    char name[256];
    struct {
        void *baseAddr;
        uint32_t size;
        int32_t prot;
    } segments[4];
    uint32_t numSegments;
    uint8_t fingerprint[20];
} OrbisKernelModuleInfo_PS5;

int sceKernelGetModuleList(int *handles, size_t max_handles, size_t *num_handles);
int sceKernelGetModuleInfo(int handle, OrbisKernelModuleInfo_PS5 *info);
int sceKernelGetAppInfo(pid_t pid, OrbisAppInfo *info);

// Overrides GoldHEN syscall 500 memory R/W for HOOK_INIT and byte patches
int sys_sdk_proc_rw(struct proc_rw *data) {
    if (!data || !data->address || !data->data || data->length == 0) {
        return -1;
    }

    uintptr_t addr = (uintptr_t)data->address;
    // Align to 16KB page boundary used by Orbis/Prospero VM
    uintptr_t page_start = addr & ~0x3FFFULL;
    size_t page_len = ((addr + data->length - page_start) + 0x3FFFULL) & ~0x3FFFULL;

    if (data->write_flags) {
        // Under PS5 kstuff / etaHEN, mprotect with RWX allows inline detour patching
        int ret = sceKernelMprotect((void *)page_start, page_len, PROT_READ | PROT_WRITE | PROT_EXEC);
        if (ret < 0) {
            // Fallback to 4KB alignment if 16KB crosses an unmapped boundary
            page_start = addr & ~0xFFFULL;
            page_len = ((addr + data->length - page_start) + 0xFFFULL) & ~0xFFFULL;
            sceKernelMprotect((void *)page_start, page_len, PROT_READ | PROT_WRITE | PROT_EXEC);
        }
        memcpy((void *)addr, data->data, data->length);
    } else {
        memcpy(data->data, (const void *)addr, data->length);
    }

    return 0;
}

// Overrides GoldHEN syscall 500 process info for eboot.bin base & Title ID
int sys_sdk_proc_info(struct proc_info *info) {
    if (!info) {
        return -1;
    }
    memset(info, 0, sizeof(struct proc_info));

    int handles[256];
    size_t num_handles = 0;
    if (sceKernelGetModuleList(handles, 256, &num_handles) < 0 || num_handles == 0) {
        return -1;
    }

    // Resolve eboot.bin (always handle index 0 or named "eboot.bin" / "executable")
    for (size_t i = 0; i < num_handles; i++) {
        OrbisKernelModuleInfo_PS5 modInfo;
        memset(&modInfo, 0, sizeof(modInfo));
        modInfo.size = sizeof(modInfo);

        if (sceKernelGetModuleInfo(handles[i], &modInfo) == 0) {
            if (i == 0 || strcmp(modInfo.name, "eboot.bin") == 0 || strcmp(modInfo.name, "executable") == 0) {
                info->base_address = (uint64_t)modInfo.segments[0].baseAddr;
                info->code_size    = modInfo.segments[0].size;
                if (modInfo.numSegments > 1) {
                    info->data_address = (uint64_t)modInfo.segments[1].baseAddr;
                    info->data_size    = modInfo.segments[1].size;
                }
                strncpy(info->name, modInfo.name, sizeof(info->name) - 1);
                break;
            }
        }
    }

    OrbisAppInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    if (sceKernelGetAppInfo(getpid(), &appInfo) == 0) {
        strncpy(info->titleid, appInfo.TitleId, sizeof(info->titleid) - 1);
    }

    // Rock Band 4 Deluxe requires 02.21; populate default version string if needed
    strncpy(info->version, "02.21", sizeof(info->version) - 1);

    return (info->base_address != 0) ? 0 : -1;
}

// Overrides GoldHEN syscall 500 klog
void sys_sdk_log(const char *fmt) {
    if (fmt) {
        sceKernelDebugOutText(0, fmt);
    }
}
