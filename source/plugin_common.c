#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include "plugin_common.h"
#include <fcntl.h>
#include <unistd.h>

static uint64_t base_address = 0;

// shadPS4 base address (new)
#define SHADPS4_BASE 0x800000000

int sceKernelGetModuleList(int *handles, size_t max_handles, size_t *num_handles);

// PS5 compatibility override for GoldHEN SDK memory R/W (syscall 500 replacement)
int sys_sdk_proc_rw(struct proc_rw *data) {
    if (!data || !data->address || !data->data || data->length == 0) {
        return -1;
    }

    uintptr_t addr = (uintptr_t)data->address;
    uintptr_t page_start = addr & ~0x3FFFULL;
    size_t page_len = ((addr + data->length - page_start) + 0x3FFFULL) & ~0x3FFFULL;

    if (data->write_flags) {
        if (sceKernelMprotect((void *)page_start, page_len, VM_PROT_ALL) < 0) {
            page_start = addr & ~0xFFFULL;
            page_len = ((addr + data->length - page_start) + 0xFFFULL) & ~0xFFFULL;
            sceKernelMprotect((void *)page_start, page_len, VM_PROT_ALL);
        }
        memcpy((void *)addr, data->data, data->length);
    } else {
        memcpy(data->data, (const void *)addr, data->length);
    }
    return 0;
}

// PS5 compatibility override for GoldHEN SDK proc_info (syscall 500 replacement)
int sys_sdk_proc_info(struct proc_info *info) {
    if (!info) {
        return -1;
    }
    memset(info, 0, sizeof(struct proc_info));

    uint64_t base = get_base_address();
    if (base == 0) {
        return -1;
    }

    info->base_address = base;
    strncpy(info->name, "eboot.bin", sizeof(info->name) - 1);
    strncpy(info->version, "02.21", sizeof(info->version) - 1);
    return 0;
}

uint64_t get_base_address() {
    if (base_address != 0) {
        return base_address;
    }

    OrbisKernelModuleInfoEx info;
    memset(&info, 0, sizeof(info));
    info.size = sizeof(info);

    OrbisAppInfo appinfo;
    memset(&appinfo, 0, sizeof(appinfo));

    bool found_app = false;

    // 1. On PS5 and PS4, check the current process PID directly first
    if (sceKernelGetAppInfo(getpid(), &appinfo) == 0 && appinfo.TitleId[0] != '\0') {
        found_app = true;
        is_emu = false;
    } else {
        // 2. Fallback scan if getpid() didn't populate TitleId
        for (pid_t pid = 0; pid <= 4096; pid++) {
            memset(&appinfo, 0, sizeof(appinfo));
            if (sceKernelGetAppInfo(pid, &appinfo) < 0)
                continue;

            if (appinfo.TitleId[0] != '\0') {
                if (strcmp(appinfo.TitleId, "CUSA02084") == 0 || strcmp(appinfo.TitleId, "CUSA02901") == 0) {
                    found_app = true;
                    is_emu = false;
                    break;
                }
            }
        }
    }

    if (found_app) {
        if (strcmp(appinfo.TitleId, "CUSA02084") == 0) {
            final_printf("US Rock Band 4 Detected!\n");
        } else if (strcmp(appinfo.TitleId, "CUSA02901") == 0) {
            final_printf("EU Rock Band 4 Detected!\n");
        } else {
            final_printf("Game loaded is not Rock Band 4! (%s)\n", appinfo.TitleId);
            return 0;
        }
    } else {
        is_emu = true;
    }

    // 3. Resolve eboot.bin base address via module list first (reliable on PS5)
    int handles[256];
    size_t num_handles = 0;
    if (!is_emu && sceKernelGetModuleList(handles, 256, &num_handles) == 0 && num_handles > 0) {
        for (size_t i = 0; i < num_handles; i++) {
            memset(&info, 0, sizeof(info));
            info.size = sizeof(info);
            if (sceKernelGetModuleInfoInternal(handles[i], &info) == 0) {
                if (i == 0 || strcmp(info.name, "eboot.bin") == 0 || strcmp(info.name, "executable") == 0) {
                    base_address = (uint64_t)info.segmentInfo[0].address;
                    if (base_address != 0) {
                        return base_address;
                    }
                }
            }
        }
    }

    // 4. Fallback module ID scan (PS4 / ShadPS4)
    for (int moduleId = 0; moduleId < 1024; moduleId++) {
        memset(&info, 0, sizeof(info));
        info.size = sizeof(info);
        if (sceKernelGetModuleInfoInternal(moduleId, &info) < 0)
            continue;

        if (is_emu) {
            if (strcmp(info.name, "rockband_ps4_s.sprx") == 0) {
                base_address = (uint64_t)info.segmentInfo[0].address;
                break;
            }
        }

        if (strcmp(info.name, "eboot.bin") == 0) {
            base_address = (uint64_t)info.segmentInfo[0].address;
            break;
        }
    }

    return base_address;
}

bool file_exists(const char* filename) {
    struct stat buff;
    return stat(filename, &buff) == 0 ? true : false;
}

float read_file_as_float(const char* filename) {
    float out;
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return 0.0f;
    }

    char string[100];
    ssize_t bytes_read = read(fd, string, sizeof(string) - 1);
    if (bytes_read > 0) {
        string[bytes_read] = '\0';

        for (ssize_t i = 0; i < bytes_read; i++) {
            if (string[i] == '\n' || string[i] == '\r') {
                string[i] = '\0';
                break;
            }
        }

        out = atof(string);
    } else {
        out = 0.0f;
    }

    close(fd);
    return out;
}

variable variables[100];
int num_vars = 0;
symvariable symvariables[100];
int num_symvars = 0;

void set_plugin_var(const char* name, int value) {
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    if (num_vars < 100) {
        strcpy(variables[num_vars].name, name);
        variables[num_vars].value = value;
        num_vars++;
    }
}

int get_plugin_var(const char* name) {
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    return 0;
}

void set_plugin_symvar(const char* name, Symbol value) {
    for (int i = 0; i < num_symvars; i++) {
        if (strcmp(symvariables[i].name, name) == 0) {
            symvariables[i].value = value;
            return;
        }
    }
    if (num_symvars < 100) {
        strcpy(symvariables[num_symvars].name, name);
        symvariables[num_symvars].value = value;
        num_symvars++;
    }
}

Symbol get_plugin_symvar(const char* name) {
    for (int i = 0; i < num_symvars; i++) {
        if (strcmp(symvariables[i].name, name) == 0) {
            return symvariables[i].value;
        }
    }

    Symbol empty = { .sym = "" };
    return empty;
}
