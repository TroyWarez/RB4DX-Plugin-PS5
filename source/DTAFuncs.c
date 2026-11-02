/*
    DTAFuncs.c - RB4DX
    code for creating DTA Functions for RB4DX.
    Licensed under the GNU Lesser General Public License version 2.1, or later.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include "plugin_common.h"
#include "rb4/data.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int cp(const char* to, const char* from)
{
    int fd_to, fd_from;
    static char buf[65536];
    ssize_t nread;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_to < 0) {
        close(fd_from);
        return -1;
    }

    while ((nread = read(fd_from, buf, sizeof(buf))) > 0)
    {
        char* out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            } else {
                close(fd_to);
                close(fd_from);
                return -1;
            }
        } while (nread > 0);
    }

    close(fd_to);
    close(fd_from);
    return (nread == 0) ? 0 : -1;
}

const char* DXFolder = "/data/HyperHEN/RB4DX/";
Symbol(*Symbol_Ctor)(Symbol*, const char*);
void(*DataInitFuncs)();
void(*DataRegisterFunc)(Symbol, DataFunc);
DataNode* (*DataArrayEvaluate)(DataNode*, DataArray*, size_t);
DataNode* (*DataNodeEvaluate)(DataNode*, DataNode*);
Symbol(*DataNodeForceSym)(DataNode*, DataArray*);
int (*DataNodeInt)(DataNode*, DataArray*);
float (*DataNodeFloat)(DataNode*, DataArray*);
SystemOptions* options;

DataNode* DataFileRename(DataNode* ret, DataArray* args) {
    final_printf("renaming file!\n");
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
    sceKernelRename(firstArg, secondArg);
    final_printf("from %s\n", firstArg);
    final_printf("to %s\n", secondArg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileRename(DataNode* ret, DataArray* args) {
    final_printf("renaming file!\n");
    char __firstArg[2048] = { 0 };
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    strcat(__firstArg, DXFolder);
    strcat(__firstArg, firstArgsym.sym);
    char* firstArg = __firstArg;

    char __secondArg[2048] = { 0 };
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    strcat(__secondArg, DXFolder);
    strcat(__secondArg, secondArgsym.sym);
    char* secondArg = __secondArg;

    sceKernelRename(firstArg, secondArg);
    final_printf("from %s\n", firstArg);
    final_printf("to %s\n", secondArg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileExists(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    ret->mType = kDataInt;
    ret->mValue.value = file_exists(Arg);
    return ret;
}

DataNode* DxFileDelete(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    sceKernelUnlink(Arg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileMkdir(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    sceKernelMkdir(Arg, 0777);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* FileRmdir(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    char* Arg = Argsym.sym;
    sceKernelRmdir(Arg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileRmdir(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    sceKernelRmdir(Arg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxWriteNullFile(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    int file = open(Arg, O_CREAT | O_WRONLY, 0777);
    if (file >= 0) close(file);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataFileCopy(DataNode* ret, DataArray* args) {
    final_printf("Copying file!\n");
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
    if (file_exists(firstArg)) {
        cp(secondArg, firstArg);
        final_printf("from %s\n", firstArg);
        final_printf("to %s\n", secondArg);
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileCopy(DataNode* ret, DataArray* args) {
    final_printf("Copying file!\n");
    char __firstArg[2048] = { 0 };
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    strcat(__firstArg, DXFolder);
    strcat(__firstArg, firstArgsym.sym);
    for (int i = 0, j; __firstArg[i] != '\0'; ++i) {
        while ((__firstArg[i] == '\'') && !(__firstArg[i] == '\0')) {
            for (j = i; __firstArg[j] != '\0'; ++j) {
                __firstArg[j] = __firstArg[j + 1];
            }
            __firstArg[j] = '\0';
        }
    }
    char* firstArg = __firstArg;

    char __secondArg[2048] = { 0 };
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    strcat(__secondArg, DXFolder);
    strcat(__secondArg, secondArgsym.sym);
    char* secondArg = __secondArg;
    if (file_exists(firstArg)) {
        cp(secondArg, firstArg);
        final_printf("from %s\n", firstArg);
        final_printf("to %s\n", secondArg);
    }
    else {
        final_printf("error finding %s\n", firstArg);
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* GetAudioCalibration(DataNode* ret, DataArray* args) {
    ret->mType = kDataInt;
    ret->mValue.value = (options != NULL) ? options->mAudioOffset : 0;
    return ret;
}

DataNode* GetVideoCalibration(DataNode* ret, DataArray* args) {
    ret->mType = kDataInt;
    ret->mValue.value = (options != NULL) ? options->mVideoOffset : 0;
    return ret;
}

DataNode* SetAudioCalibration(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    float Argfloat = DataNodeFloat(&_Arg, args);
    if (options != NULL) {
        options->mAudioOffset = Argfloat;
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* SetVideoCalibration(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    float Argfloat = DataNodeFloat(&_Arg, args);
    if (options != NULL) {
        options->mVideoOffset = Argfloat;
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxForceSym(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Arg = DataNodeForceSym(&_Arg, args);
    ret->mType = kDataSymbol;
    ret->mValue.symbol = Arg.sym;
    return ret;
}

DataNode* DataIsEmulator(DataNode* ret, DataArray* args) {
    ret->mType = kDataInt;
    ret->mValue.value = is_emu ? 1 : 0;
    return ret;
}

DataNode* DataIsNeo(DataNode* ret, DataArray* args) {
    ret->mType = kDataInt;
    ret->mValue.value = sceKernelHasNeoMode();
    return ret;
}

typedef union _FloatInt {
    float f;
    unsigned int i;
} FloatInt;

uint32_t swap_endian(uint32_t value) {
    return ((value >> 24) & 0x000000FF) |
        ((value >> 8) & 0x0000FF00) |
        ((value << 8) & 0x00FF0000) |
        ((value << 24) & 0xFF000000);
}

DataNode* DataFloatToInt(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    float Argfloat = DataNodeFloat(&_Arg, args);
    FloatInt convert;
    convert.f = Argfloat;
    ret->mType = kDataInt;
    ret->mValue.value = swap_endian(convert.i);
    return ret;
}

void replace_floats(const char* filename, long offset, const float* values, size_t count) {
    int fp = open(filename, O_RDWR);
    if (fp < 0) return;
    lseek(fp, offset, SEEK_SET);
    write(fp, values, sizeof(float) * count);
    close(fp);
}

const char* score_file_path = "/data/HyperHEN/RB4DX/ps4/ui/game/hud.scene_ps4";
const char* countdown_file_path = "/data/HyperHEN/RB4DX/ps4/ui/game/break_countdown.entity_ps4";
const char* solo_file_path = "/data/HyperHEN/RB4DX/ps4/ui/game/solo_percentage.entity_ps4";
const char* mtv_file_path = "/data/HyperHEN/RB4DX/ps4/ui/game/song_artist_overlay.entity_ps4";

DataNode* DataWriteScoreFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    float firstArg = DataNodeFloat(&_firstArg, args);

    DataNode _secondArg = (args->mNodes->n[2]);
    float secondArg = DataNodeFloat(&_secondArg, args);

    DataNode _thirdArg = (args->mNodes->n[3]);
    float thirdArg = DataNodeFloat(&_thirdArg, args);

    DataNode _fourthArg = (args->mNodes->n[4]);
    int fourthArg = DataNodeInt(&_fourthArg, args);

    float pos[3] = { firstArg, secondArg, thirdArg };

    replace_floats(score_file_path, 0x31a3, pos, 3);

    int binaryfile = open(score_file_path, O_RDWR);
    if (binaryfile >= 0) {
        char byte = (fourthArg == 0) ? 'i' : 'n';
        lseek(binaryfile, 0x1ec7a, SEEK_SET);
        write(binaryfile, &byte, 1);
        close(binaryfile);
    }

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteCountdownFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    float firstArg = DataNodeFloat(&_firstArg, args);

    DataNode _secondArg = (args->mNodes->n[2]);
    float secondArg = DataNodeFloat(&_secondArg, args);
    float secondArg2 = secondArg + 10;

    DataNode _thirdArg = (args->mNodes->n[3]);
    float thirdArg = DataNodeFloat(&_thirdArg, args);

    float posa[3] = { firstArg, secondArg2, thirdArg };
    float posb[3] = { firstArg, secondArg, thirdArg };

    replace_floats(countdown_file_path, 0xCBA, posa, 3);
    replace_floats(countdown_file_path, 0xE36, posb, 3);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteVoxCountdownFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    float firstArg = DataNodeFloat(&_firstArg, args);

    DataNode _secondArg = (args->mNodes->n[2]);
    float secondArg = DataNodeFloat(&_secondArg, args);

    DataNode _thirdArg = (args->mNodes->n[3]);
    float thirdArg = DataNodeFloat(&_thirdArg, args);

    float pos[3] = { firstArg, secondArg, thirdArg };

    replace_floats(score_file_path, 0x440F, pos, 3);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteMTVFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    float firstArg = DataNodeFloat(&_firstArg, args);
    float firstArg2 = firstArg + 3.04834747314453;

    DataNode _secondArg = (args->mNodes->n[2]);
    float secondArg = DataNodeFloat(&_secondArg, args);

    DataNode _thirdArg = (args->mNodes->n[3]);
    float thirdArg = DataNodeFloat(&_thirdArg, args);
    float thirdArg2;

    DataNode _fourthArg = (args->mNodes->n[4]);
    int fourthArg = DataNodeInt(&_fourthArg, args);

    DataNode _fifthArg = (args->mNodes->n[5]);
    int fifthArg = DataNodeInt(&_fifthArg, args);

    if (fourthArg == 0)
        thirdArg2 = thirdArg - 38.2138786;
    else
        thirdArg2 = thirdArg - 65.21386337280316;

    float posa[3] = { firstArg, secondArg, thirdArg };
    float posb[3] = { firstArg2, secondArg, thirdArg2 };

    replace_floats(mtv_file_path, 0x631, posa, 3);
    replace_floats(mtv_file_path, 0x821, posb, 3);

    int binaryfile = open(mtv_file_path, O_RDWR);
    if (binaryfile >= 0) {
        char byte = (fifthArg == 0) ? 'o' : 'n';
        lseek(binaryfile, 0xc64, SEEK_SET);
        write(binaryfile, &byte, 1);
        byte = (fifthArg == 0) ? 'i' : 'n';
        lseek(binaryfile, 0xce6, SEEK_SET);
        write(binaryfile, &byte, 1);
        close(binaryfile);
    }

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteSoloFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    float firstArg = DataNodeFloat(&_firstArg, args);

    DataNode _secondArg = (args->mNodes->n[2]);
    float secondArg = DataNodeFloat(&_secondArg, args);

    DataNode _thirdArg = (args->mNodes->n[3]);
    float thirdArg = DataNodeFloat(&_thirdArg, args);

    float pos[3] = { firstArg, secondArg, thirdArg };

    replace_floats(solo_file_path, 0x130C, pos, 3);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteVoxSoloFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    float firstArg = DataNodeFloat(&_firstArg, args);

    DataNode _secondArg = (args->mNodes->n[2]);
    float secondArg = DataNodeFloat(&_secondArg, args);

    DataNode _thirdArg = (args->mNodes->n[3]);
    float thirdArg = DataNodeFloat(&_thirdArg, args);

    float pos[3] = { firstArg, secondArg, thirdArg };

    replace_floats(score_file_path, 0x3887, pos, 3);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataReplaceFloat(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = (args->mNodes->n[2]);
    int secondArg = DataNodeInt(&_secondArg, args);
    DataNode _thirdArg = (args->mNodes->n[3]);
    float thirdArg = DataNodeFloat(&_thirdArg, args);

    replace_floats(firstArg, secondArg, &thirdArg, 1);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataReplaceByte(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = (args->mNodes->n[2]);
    int secondArg = DataNodeInt(&_secondArg, args);
    DataNode _thirdArg = (args->mNodes->n[3]);
    int thirdArg = DataNodeInt(&_thirdArg, args);

    int binaryfile = open(firstArg, O_RDWR);
    if (binaryfile >= 0) {
        unsigned char byte = thirdArg;
        lseek(binaryfile, secondArg, SEEK_SET);
        write(binaryfile, &byte, 1);
        close(binaryfile);
    }

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteBinaryFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;

    int fd = open(firstArg, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd >= 0) {
        char* p;
        for (p = secondArg; *p != '\0'; ) {
            unsigned int x;
            int n;
            if (sscanf(p, "%2x%n", &x, &n) != 1) break;
            unsigned char b = (unsigned char)x;
            write(fd, &b, 1);
            p += n;
        }
        close(fd);
    }

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataReadFileAsFloat(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    char* Arg = Argsym.sym;
    ret->mType = kDataFloat;
    ret->mValue.real = read_file_as_float(Arg);
    return ret;
}

DataNode* DxReadFileAsFloat(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcpy(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    ret->mType = kDataFloat;
    ret->mValue.real = read_file_as_float(Arg);
    return ret;
}

DataNode* DataSetPluginVar(DataNode* ret, DataArray* args) {
    DataNode _Arg1 = (args->mNodes->n[1]);
    DataNode _Arg2 = (args->mNodes->n[2]);
    Symbol Argsym = DataNodeForceSym(&_Arg1, args);
    char* Arg1 = Argsym.sym;
    int Arg2 = DataNodeInt(&_Arg2, args);
    set_plugin_var(Arg1, Arg2);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataGetPluginVar(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    char* Arg = Argsym.sym;
    ret->mType = kDataInt;
    ret->mValue.value = get_plugin_var(Arg);
    return ret;
}

DataNode* DataSetPluginSymVar(DataNode* ret, DataArray* args) {
    DataNode _Arg1 = (args->mNodes->n[1]);
    DataNode _Arg2 = (args->mNodes->n[2]);
    Symbol Argsym = DataNodeForceSym(&_Arg1, args);
    char* Arg1 = Argsym.sym;
    Symbol Arg2 = DataNodeForceSym(&_Arg2, args);
    set_plugin_symvar(Arg1, Arg2);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataGetPluginSymVar(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    char* Arg = Argsym.sym;
    ret->mType = kDataSymbol;
    ret->mValue.symbol = get_plugin_symvar(Arg).sym;
    return ret;
}

HOOK_INIT(DataInitFuncs);

void DataInitFuncs_hook() {
    Symbol funcsym;
    Symbol_Ctor(&funcsym, "file_rename");
    DataRegisterFunc(funcsym, DataFileRename);
    Symbol_Ctor(&funcsym, "file_move");
    DataRegisterFunc(funcsym, DataFileRename);
    Symbol_Ctor(&funcsym, "file_copy");
    DataRegisterFunc(funcsym, DataFileCopy);
    Symbol_Ctor(&funcsym, "dx_file_copy");
    DataRegisterFunc(funcsym, DxFileCopy);
    Symbol_Ctor(&funcsym, "dx_file_rename");
    DataRegisterFunc(funcsym, DxFileRename);
    Symbol_Ctor(&funcsym, "dx_file_exists");
    DataRegisterFunc(funcsym, DxFileExists);
    Symbol_Ctor(&funcsym, "dx_file_delete");
    DataRegisterFunc(funcsym, DxFileDelete);
    Symbol_Ctor(&funcsym, "dx_file_mkdir");
    DataRegisterFunc(funcsym, DxFileMkdir);
    Symbol_Ctor(&funcsym, "file_rmdir");
    DataRegisterFunc(funcsym, FileRmdir);
    Symbol_Ctor(&funcsym, "dx_file_rmdir");
    DataRegisterFunc(funcsym, DxFileRmdir);
    Symbol_Ctor(&funcsym, "dx_write_null_file");
    DataRegisterFunc(funcsym, DxWriteNullFile);
    Symbol_Ctor(&funcsym, "force_sym");
    DataRegisterFunc(funcsym, DxForceSym);
    Symbol_Ctor(&funcsym, "is_emu");
    DataRegisterFunc(funcsym, DataIsEmulator);
    Symbol_Ctor(&funcsym, "is_neo");
    DataRegisterFunc(funcsym, DataIsNeo);
    Symbol_Ctor(&funcsym, "is_ps4_pro");
    DataRegisterFunc(funcsym, DataIsNeo);
    Symbol_Ctor(&funcsym, "float_to_int");
    DataRegisterFunc(funcsym, DataFloatToInt);
    Symbol_Ctor(&funcsym, "read_file_as_float");
    DataRegisterFunc(funcsym, DataReadFileAsFloat);
    Symbol_Ctor(&funcsym, "dx_read_file_as_float");
    DataRegisterFunc(funcsym, DxReadFileAsFloat);
    Symbol_Ctor(&funcsym, "write_binary_file");
    DataRegisterFunc(funcsym, DataWriteBinaryFile);
    Symbol_Ctor(&funcsym, "write_score_file");
    DataRegisterFunc(funcsym, DataWriteScoreFile);
    Symbol_Ctor(&funcsym, "write_countdown_file");
    DataRegisterFunc(funcsym, DataWriteCountdownFile);
    Symbol_Ctor(&funcsym, "write_solo_file");
    DataRegisterFunc(funcsym, DataWriteSoloFile);
    Symbol_Ctor(&funcsym, "write_vox_countdown_file");
    DataRegisterFunc(funcsym, DataWriteVoxCountdownFile);
    Symbol_Ctor(&funcsym, "write_vox_solo_file");
    DataRegisterFunc(funcsym, DataWriteVoxSoloFile);
    Symbol_Ctor(&funcsym, "write_mtv_file");
    DataRegisterFunc(funcsym, DataWriteMTVFile);
    Symbol_Ctor(&funcsym, "replace_float");
    DataRegisterFunc(funcsym, DataReplaceFloat);
    Symbol_Ctor(&funcsym, "replace_byte");
    DataRegisterFunc(funcsym, DataReplaceByte);
    Symbol_Ctor(&funcsym, "get_audio_calibration");
    DataRegisterFunc(funcsym, GetAudioCalibration);
    Symbol_Ctor(&funcsym, "get_video_calibration");
    DataRegisterFunc(funcsym, GetVideoCalibration);
    Symbol_Ctor(&funcsym, "set_audio_calibration");
    DataRegisterFunc(funcsym, SetAudioCalibration);
    Symbol_Ctor(&funcsym, "set_video_calibration");
    DataRegisterFunc(funcsym, SetVideoCalibration);
    Symbol_Ctor(&funcsym, "set_plugin_var");
    DataRegisterFunc(funcsym, DataSetPluginVar);
    Symbol_Ctor(&funcsym, "get_plugin_var");
    DataRegisterFunc(funcsym, DataGetPluginVar);
    Symbol_Ctor(&funcsym, "set_plugin_symvar");
    DataRegisterFunc(funcsym, DataSetPluginSymVar);
    Symbol_Ctor(&funcsym, "get_plugin_symvar");
    DataRegisterFunc(funcsym, DataGetPluginSymVar);

    HOOK_CONTINUE(DataInitFuncs, void (*)());
}

void (*SystemOptionsLoad)(SystemOptions*, void*);

HOOK_INIT(SystemOptionsLoad);

void SystemOptionsLoad_hook(SystemOptions* thisoptions, void* binstream) {
    options = thisoptions;
    HOOK_CONTINUE(SystemOptionsLoad, void (*)(SystemOptions*, void*), thisoptions, binstream);
    if (file_exists("/data/HyperHEN/RB4DX/videocalib.ini")) {
        options->mVideoOffset = read_file_as_float("/data/HyperHEN/RB4DX/videocalib.ini");
    }
    if (file_exists("/data/HyperHEN/RB4DX/audiocalib.ini")) {
        options->mAudioOffset = read_file_as_float("/data/HyperHEN/RB4DX/audiocalib.ini");
    }
    return;
}

void (*RBSystemOptionsSave)(void*, void*);

HOOK_INIT(RBSystemOptionsSave);

void RBSystemOptionsSave_hook(void* thisoptions, void* binstream) {
    HOOK_CONTINUE(RBSystemOptionsSave, void (*)(void*, void*), thisoptions, binstream);
    if (file_exists("/data/HyperHEN/RB4DX/videocalib.ini")) {
        sceKernelUnlink("/data/HyperHEN/RB4DX/videocalib.ini");
    }
    if (file_exists("/data/HyperHEN/RB4DX/audiocalib.ini")) {
        sceKernelUnlink("/data/HyperHEN/RB4DX/audiocalib.ini");
    }
    return;
}

bool (*SetFocusByData)(void*, Symbol, DataNode*);
bool (*SetFocusByDataIndex)(void*, int, bool);
HOOK_INIT(SetFocusByData);
bool SetFocusByData_hook(void* thisUIList, Symbol focussym, DataNode* focustype) {
    if (strcmp(focussym.sym, "dx_index") == 0) {
        DataArray array;
        int index = DataNodeInt(focustype, &array);
        return SetFocusByDataIndex(thisUIList, index, false);
    }
    return HOOK_CONTINUE(SetFocusByData, bool (*)(void*, Symbol, DataNode*), thisUIList, focussym, focustype);
}

void InitDTAHooks() {
    uint64_t base_address = get_base_address();

    DataInitFuncs = (void*)(base_address + 0x00222350);
    DataRegisterFunc = (void*)(base_address + 0x002221f0);
    DataArrayEvaluate = (void*)(base_address + 0x000c7d30);
    DataNodeEvaluate = (void*)(base_address + 0x00234f10);
    DataNodeForceSym = (void*)(base_address + 0x0000e850);
    DataNodeFloat = (void*)(base_address + 0x0000ee30);
    DataNodeInt = (void*)(base_address + 0x00000eb40);
    Symbol_Ctor = (void*)(base_address + 0x00256fd0);
    SystemOptionsLoad = (void*)(base_address + 0x011b2310);
    RBSystemOptionsSave = (void*)(base_address + 0x00d667c0);
    SetFocusByData = (void*)(base_address + 0x008b9540);
    SetFocusByDataIndex = (void*)(base_address + 0x008b8560);

    HOOK(DataInitFuncs);
    HOOK(SystemOptionsLoad);
    HOOK(RBSystemOptionsSave);
    HOOK(SetFocusByData);
}

void DestroyDTAHooks() {
    UNHOOK(DataInitFuncs);
    UNHOOK(SystemOptionsLoad);
    UNHOOK(RBSystemOptionsSave);
    UNHOOK(SetFocusByData);
}
