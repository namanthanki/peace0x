#include "peace0x/platform.h"
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define PEACE0X_WINDOWS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#include <io.h>
#else
#define PEACE0X_POSIX
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#endif

int64_t platform_get_time_ms(void) {
#if defined(PEACE0X_WINDOWS)
    static LARGE_INTEGER freq;
    static int freq_init = 0;
    if (!freq_init) {
        QueryPerformanceFrequency(&freq);
        freq_init = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (int64_t)((now.QuadPart * 1000LL) / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)(ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL);
#endif
}

bool platform_input_waiting(void) {
#if defined(PEACE0X_WINDOWS)
    static int initialized = 0;
    static int is_pipe = 0;
    static HANDLE stdin_handle;
    DWORD mode = 0;

    if (!initialized) {
        initialized = 1;
        stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
        is_pipe = !GetConsoleMode(stdin_handle, &mode);
        if (!is_pipe) {
            SetConsoleMode(stdin_handle, mode & (DWORD)~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(stdin_handle);
        }
    }

    if (is_pipe) {
        DWORD bytes_avail = 0;
        if (!PeekNamedPipe(stdin_handle, NULL, 0, NULL, &bytes_avail, NULL)) {
            return true;
        }
        return bytes_avail > 0;
    } else {
        DWORD events = 0;
        GetNumberOfConsoleInputEvents(stdin_handle, &events);
        return events > 1;
    }
#else
    fd_set readfds;
    struct timeval tv = { 0, 0 };
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &readfds) != 0;
#endif
}

void platform_read_input(SearchInfo *info) {
    ASSERT(info != NULL);

    if (platform_input_waiting()) {
        char input[256] = { 0 };
#if defined(PEACE0X_WINDOWS)
        int bytes = _read(_fileno(stdin), input, sizeof(input) - 1);
#else
        ssize_t bytes = read(STDIN_FILENO, input, sizeof(input) - 1);
#endif
        if (bytes > 0) {
            input[bytes] = '\0';
            char *newline = strchr(input, '\n');
            if (newline != NULL) {
                *newline = '\0';
            }
            if (strncmp(input, "quit", 4) == 0) {
                info->quit = true;
                info->stopped = true;
            } else if (strncmp(input, "stop", 4) == 0) {
                info->stopped = true;
            } else if (strncmp(input, "isready", 7) == 0) {
                printf("readyok\n");
                fflush(stdout);
            }
        }
    }
}
