#include "amxxmodule.h"
#include <usercmd.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

constexpr int kMaxPlayers = 32;
constexpr int kHistory = 60;
constexpr float kWindow = 1.0f;
constexpr float kMaxLoss = 0.1f;

struct PlayerFps {
    int userId;
    unsigned int lastSeed;
    const usercmd_s* lastCmd;
    int frames;
    int msec;
    float start;
    int count;
    int head;
    int histFrames[kHistory];
    int histMsec[kHistory];
    float histElapsed[kHistory];
};

PlayerFps g_players[kMaxPlayers + 1];
float g_serverTime;
cvar_t* g_customSeed;

void resetPlayer(int index) {
    std::memset(&g_players[index], 0, sizeof(PlayerFps));
}

void resetWindow(int index) {
    PlayerFps& p = g_players[index];
    p.start = 0.0f;
    p.frames = 0;
    p.msec = 0;
    p.lastSeed = 0;
    p.lastCmd = nullptr;
}

int playerIndex(const edict_t* ent) {
    if (ent == nullptr) return 0;
    const int index = ENTINDEX(const_cast<edict_t*>(ent));
    return (index >= 1 && index <= kMaxPlayers) ? index : 0;
}

void claimSlot(edict_t* ent) {
    if (const int index = playerIndex(ent)) {
        const int userId = GETPLAYERUSERID(ent);
        if (g_players[index].userId != userId) {
            resetPlayer(index);
            g_players[index].userId = userId;
        } else {
            resetWindow(index);
        }
    }
}

bool windowComplete(int frames, int msec, float elapsed) {
    if (frames <= 0) return false;
    const float expected = elapsed * 1000.0f;
    const float lossSlack = kMaxLoss * expected;
    const float cmdSlack = 2.0f * msec / frames;
    return msec >= expected - (lossSlack > cmdSlack ? lossSlack : cmdSlack);
}

float average(int index, int windows) {
    const PlayerFps& p = g_players[index];
    if (windows > p.count) windows = p.count;
    if (windows <= 0) return 0.0f;

    int frames = 0;
    float elapsed = 0.0f;
    for (int i = 0; i < windows; ++i) {
        const int slot = (p.head - 1 - i + kHistory) % kHistory;
        frames += p.histFrames[slot];
        elapsed += p.histElapsed[slot];
    }
    return elapsed > 0.0f ? frames / elapsed : 0.0f;
}

float averageMsec(int index, int windows) {
    const PlayerFps& p = g_players[index];
    if (windows > p.count) windows = p.count;
    if (windows <= 0) return 0.0f;

    int frames = 0;
    int msec = 0;
    for (int i = 0; i < windows; ++i) {
        const int slot = (p.head - 1 - i + kHistory) % kHistory;
        frames += p.histFrames[slot];
        msec += p.histMsec[slot];
    }
    return msec > 0 ? frames / (msec * 0.001f) : 0.0f;
}

bool isTrackedClient(int index) {
    return MF_IsPlayerIngame(index) && !MF_IsPlayerBot(index) && !MF_IsPlayerHLTV(index);
}

bool containsNoCase(const char* haystack, const char* needle) {
    for (; *haystack; ++haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && std::tolower(static_cast<unsigned char>(*h)) == std::tolower(static_cast<unsigned char>(*n))) {
            ++h;
            ++n;
        }
        if (*n == '\0') return true;
    }
    return *needle == '\0';
}

void formatFps(char* out, size_t size, float fps) {
    if (fps == 0.0f)
        std::snprintf(out, size, "?");
    else
        std::snprintf(out, size, "%.1f", fps);
}

void cmdFpsPeek() {
    const char* filter = CMD_ARGC() > 1 ? CMD_ARGV(1) : "";
    int filterUserId = 0;
    if (filter[0] == '#') filterUserId = std::atoi(filter + 1);

    MF_PrintSrvConsole("%-4s %-7s %-8s %-8s %-8s %-8s %s\n", "id", "userid", "fps", "msec", "5s", "30s", "name");

    int shown = 0;
    const int maxClients = gpGlobals->maxClients < kMaxPlayers ? gpGlobals->maxClients : kMaxPlayers;
    for (int i = 1; i <= maxClients; ++i) {
        if (!isTrackedClient(i)) continue;

        edict_t* ent = INDEXENT(i);
        const int userId = GETPLAYERUSERID(ent);
        const char* name = MF_GetPlayerName(i);
        if (filterUserId > 0) {
            if (userId != filterUserId) continue;
        } else if (!containsNoCase(name, filter)) {
            continue;
        }

        char fps[16];
        char msec[16];
        char avg[16];
        char avgLong[16];
        formatFps(fps, sizeof(fps), average(i, 1));
        formatFps(msec, sizeof(msec), averageMsec(i, 1));
        formatFps(avg, sizeof(avg), average(i, 5));
        formatFps(avgLong, sizeof(avgLong), average(i, 30));

        MF_PrintSrvConsole("#%-3d %-7d %-8s %-8s %-8s %-8s %s\n", i, userId, fps, msec, avg, avgLong, name);
        ++shown;
    }

    MF_PrintSrvConsole("%d player(s)\n", shown);
}

bool checkPlayer(AMX* amx, int index) {
    if (index < 1 || index > gpGlobals->maxClients || index > kMaxPlayers) {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid player %d", index);
        return false;
    }
    return true;
}

cell AMX_NATIVE_CALL native_fps_get(AMX* amx, cell* params) {
    const int index = static_cast<int>(params[1]);
    if (!checkPlayer(amx, index)) return 0;
    const int windows = static_cast<int>(params[2]);
    return amx_ftoc(average(index, windows));
}

cell AMX_NATIVE_CALL native_fps_msec(AMX* amx, cell* params) {
    const int index = static_cast<int>(params[1]);
    if (!checkPlayer(amx, index)) return 0;
    const int windows = static_cast<int>(params[2]);
    return amx_ftoc(averageMsec(index, windows));
}

cell AMX_NATIVE_CALL native_fps_windows(AMX* amx, cell* params) {
    const int index = static_cast<int>(params[1]);
    if (!checkPlayer(amx, index)) return 0;
    return g_players[index].count;
}

cell AMX_NATIVE_CALL native_fps_reset(AMX* amx, cell* params) {
    const int index = static_cast<int>(params[1]);
    if (!checkPlayer(amx, index)) return 0;
    resetPlayer(index);
    return 1;
}

AMX_NATIVE_INFO g_natives[] = {
    {"fps_get", native_fps_get},
    {"fps_msec", native_fps_msec},
    {"fps_windows", native_fps_windows},
    {"fps_reset", native_fps_reset},
    {nullptr, nullptr}
};

} // namespace

void OnAmxxAttach() {
    for (int i = 0; i <= kMaxPlayers; ++i) resetPlayer(i);
    MF_AddNatives(g_natives);
    REG_SVR_COMMAND("fps_peek", cmdFpsPeek);
    g_customSeed = CVAR_GET_POINTER("sv_usercmd_custom_random_seed");
}

void CmdStart(const edict_t* ent, const usercmd_s* cmd, unsigned int randomSeed) {
    const int index = playerIndex(ent);
    if (index == 0 || (ent->v.flags & (FL_FAKECLIENT | FL_PROXY))) RETURN_META(MRES_IGNORED);

    PlayerFps& p = g_players[index];
    const bool splitHalf = randomSeed != 0 && randomSeed == p.lastSeed && cmd == p.lastCmd
        && !(g_customSeed && g_customSeed->value != 0.0f);
    p.lastSeed = randomSeed;
    p.lastCmd = cmd;

    const float now = g_serverTime;
    if (p.start == 0.0f || now < p.start) {
        p.start = now;
        p.frames = 0;
        p.msec = 0;
        RETURN_META(MRES_IGNORED);
    }

    if (randomSeed != 0) {
        if (!splitHalf) ++p.frames;
        p.msec += cmd->msec;
    }
    const float elapsed = now - p.start;
    if (elapsed >= kWindow) {
        if (windowComplete(p.frames, p.msec, elapsed)) {
            p.histFrames[p.head] = p.frames;
            p.histElapsed[p.head] = elapsed;
            p.histMsec[p.head] = p.msec;
            p.head = (p.head + 1) % kHistory;
            if (p.count < kHistory) ++p.count;
        }
        p.frames = 0;
        p.msec = 0;
        p.start = now;
    }
    RETURN_META(MRES_IGNORED);
}

void StartFrame() {
    g_serverTime = gpGlobals->time;
    RETURN_META(MRES_IGNORED);
}

BOOL ClientConnect(edict_t* ent, const char*, const char*, char*) {
    claimSlot(ent);
    RETURN_META_VALUE(MRES_IGNORED, TRUE);
}

void ClientPutInServer(edict_t* ent) {
    claimSlot(ent);
    RETURN_META(MRES_IGNORED);
}

void ClientDisconnect(edict_t* ent) {
    if (const int index = playerIndex(ent)) resetWindow(index);
    RETURN_META(MRES_IGNORED);
}
