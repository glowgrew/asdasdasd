#include <cstdio>
#include <cstdlib>
#include <tchar.h>
#include <windows.h>
#include <cstdint>
#include <windowsx.h>
#include <fstream>
#include <iostream>

#define KEY_SHIFTED 0x8000
#define CLRCNG 15
#define FILENAME "config.conf"

struct COLOR {
    int r = 0;
    int g = 0;
    int b = 0;
};

COLOR bgColor = {0, 0, 255}; // цвет фона окна
COLOR lineColor = {255, 0, 0}; // цвет сетки
long matrixSize = 10; // N
int dbMode = 4; // режим считывания конфигурационного файла
LONG wWidth = 320; // ширина окна
LONG wHeight = 240; // высота окна
LPTSTR matrix;
UINT syncMsg = RegisterWindowMessage("Game Field Update");

const TCHAR szWinClass[] = _T("Painter App Window");
const TCHAR szWinName[] = _T("Painter App Window");
const TCHAR szSharedMemoryName[] = _T("Shared Painter App Memory");

HWND hwnd; /* This is the handle for our window */
HBRUSH hBrush; /* Current brush */

/* Runs Notepad */
void RunNotepad() {
    STARTUPINFO sInfo;

    PROCESS_INFORMATION pInfo;

    ZeroMemory(&sInfo, sizeof(STARTUPINFO));

    puts("Starting Notepad...");
    CreateProcess(_T("C:\\Windows\\Notepad.exe"),
                  nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &sInfo, &pInfo);
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;
    int x;
    int y;
    HBRUSH hTempBrush;

    RECT w;
    GetClientRect(hwnd, &w);
    int cWidth = w.right - w.left;
    int cHeight = w.bottom - w.top;

    /* handle the messages */
    switch (message) {
        case WM_LBUTTONUP: {
            x = GET_X_LPARAM(lParam);
            y = GET_Y_LPARAM(lParam);
            if (matrix[x / (cWidth / matrixSize) * matrixSize + y / (cHeight / matrixSize)] == 0) {
                matrix[x / (cWidth / matrixSize) * matrixSize + y / (cHeight / matrixSize)] = 1;
            }
            PostMessage(HWND_BROADCAST, syncMsg, NULL, NULL);
            // InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case WM_RBUTTONUP: {
            x = GET_X_LPARAM(lParam);
            y = GET_Y_LPARAM(lParam);
            if (matrix[x / (cWidth / matrixSize) * matrixSize + y / (cHeight / matrixSize)] == 0) {
                matrix[x / (cWidth / matrixSize) * matrixSize + y / (cHeight / matrixSize)] = 2;
            }
            PostMessage(HWND_BROADCAST, syncMsg, NULL, NULL);
            // InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            const byte r = lineColor.r, g = lineColor.g, b = lineColor.b;
            const bool r255 = r == 255, b255 = b == 255, g255 = g == 255;
            const bool r0 = r == 0, b0 = b == 0, g0 = g == 0;
            if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
                if (r255 && g0 && !b255) lineColor.b += CLRCNG;
                else if (!r0 && b255 && g0) lineColor.r -= CLRCNG;
                else if (b255 && r0 && !g255) lineColor.g += CLRCNG;
                else if (!b0 && g255 && r0) lineColor.b -= CLRCNG;
                else if (g255 && b0 && !r255) lineColor.r += CLRCNG;
                else if (r255 && b0 && !g0) lineColor.g -= CLRCNG;
            }
            else {
                if (r255 && b0 && !g255) lineColor.g += CLRCNG;
                else if (!r0 && g255 && b0) lineColor.r -= CLRCNG;
                else if (g255 && r0 && !b255) lineColor.b += CLRCNG;
                else if (!g0 && b255 && r0) lineColor.g -= CLRCNG;
                else if (b255 && g0 && !r255) lineColor.r += CLRCNG;
                else if (r255 && !b0 && g0) lineColor.b -= CLRCNG;
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        case WM_KEYDOWN: {
            switch (wParam) {
                case 67:
                    if (GetKeyState(VK_SHIFT) & KEY_SHIFTED) {
                        RunNotepad();
                    }
                    return 0;
                case 81:
                case VK_ESCAPE: {
                    PostQuitMessage(0);
                    return 0;
                }
                case VK_RETURN: {
                    bgColor = {(uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256)};
                    hBrush = CreateSolidBrush(RGB(bgColor.r, bgColor.g, bgColor.b));
                    hTempBrush = (HBRUSH)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
                    DeleteObject(hTempBrush);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    return 0;
                }
            }
        }
        case WM_SIZE: {
            GetWindowRect(hwnd, &w);
            wWidth = w.right - w.left;
            wHeight = w.bottom - w.top;
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_PAINT: {
            hdc = BeginPaint(hwnd, &ps);
            RECT wSize;
            GetClientRect(hwnd, &wSize);
            FillRect(hdc, &wSize, hBrush);
            HPEN hPen = CreatePen(PS_SOLID, NULL, COLORREF(RGB(lineColor.r, lineColor.g, lineColor.b)));
            HPEN hDefaultPen = (HPEN)SelectObject(hdc, hPen);
            for (long i = 0; i < matrixSize; i++) {
                MoveToEx(hdc, i * cWidth / matrixSize, 0, nullptr);
                LineTo(hdc, i * cWidth / matrixSize, cHeight);
                MoveToEx(hdc, 0, i * cHeight / matrixSize, nullptr);
                LineTo(hdc, cWidth, i * cHeight / matrixSize);
            }
            hPen = (HPEN)SelectObject(hdc, hDefaultPen);
            DeleteObject(hPen);

            hPen = CreatePen(PS_SOLID, NULL, RGB(0, 0, 0));

            HBRUSH hDefaultBrush = (HBRUSH)SelectObject(hdc, hBrush);
            for (int i = 0; i < matrixSize; i++) {
                for (int j = 0; j < matrixSize; j++) {
                    if (matrix[i * matrixSize + j] == 1) {
                        Ellipse(hdc, i * cWidth / matrixSize, j * cHeight / matrixSize, (i + 1) * cWidth / matrixSize,
                                (j + 1) * cHeight / matrixSize);
                    }
                    if (matrix[i * matrixSize + j] == 2) {
                        MoveToEx(hdc, i * cWidth / matrixSize, j * cHeight / matrixSize, nullptr);
                        LineTo(hdc, (i + 1) * cWidth / matrixSize, (j + 1) * cHeight / matrixSize);
                        MoveToEx(hdc, i * cWidth / matrixSize, (j + 1) * cHeight / matrixSize, nullptr);
                        LineTo(hdc, (i + 1) * cWidth / matrixSize, j * cHeight / matrixSize);
                    }
                }
            }

            DeleteObject(hPen);
            DeleteObject(hDefaultBrush);

            EndPaint(hwnd, &ps);
            return 0;
        }
        default: {
            if (message == syncMsg) InvalidateRect(hwnd, nullptr, TRUE);
            break;
        }
    }

    /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void read_ifstream() {
    std::cout << "Reading using ifstream" << std::endl;
    std::ifstream fin;
    fin.open(FILENAME);
    if (fin) {
        fin >> matrixSize >> wWidth >> wHeight >>
                bgColor.r >> bgColor.g >> bgColor.b >> lineColor.r >> lineColor.g >> lineColor.b;
    }
    fin.close();
}

void write_ifstream() {
    std::cout << "Writing using ifstream" << std::endl;
    std::ofstream fout;
    fout.open(FILENAME, std::ofstream::out | std::ofstream::trunc);
    fout << matrixSize << "\n" << wWidth << "\n" << wHeight << "\n"
            << bgColor.r << "\n" << bgColor.g << "\n" << bgColor.b << "\n"
            << lineColor.r << "\n" << lineColor.g << "\n" << lineColor.b << "\n";
    fout.close();
}

void read_memory() {
    read_ifstream();
    // implement
}

void write_memory() {
    write_ifstream();
    // implement
}

void write_fopen() {
    std::cout << "Writing using fopen" << std::endl;
    FILE* file;
    fopen_s(&file, FILENAME, "w");
    if (file == nullptr) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }
    fprintf(file, "%ld\n%ld\n%ld\n%d\n%d\n%d\n%d\n%d\n%d", matrixSize, wWidth, wHeight,
            bgColor.r, bgColor.g, bgColor.b, lineColor.r, lineColor.g, lineColor.b);
    fclose(file);
}

void write_native() {
    std::cout << "Writing using native WinAPI" << std::endl;

    HANDLE hFile = CreateFile(FILENAME, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    char buffer[256];
    sprintf_s(buffer, sizeof(buffer), "%ld\n%ld\n%ld\n%d\n%d\n%d\n%d\n%d\n%d", matrixSize, wWidth, wHeight,
              bgColor.r, bgColor.g, bgColor.b, lineColor.r, lineColor.g, lineColor.b);

    DWORD bytesWritten;
    if (!WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, nullptr)) {
        std::cerr << "Error writing file." << std::endl;
    }

    CloseHandle(hFile);
}

void read_fopen() {
    std::cout << "Reading using fopen" << std::endl;
    FILE* file = fopen(FILENAME, "r");
    if (file == nullptr) {
        return;
    }
    fscanf(file, "%ld\n%ld\n%ld\n%d\n%d\n%d\n%d\n%d\n%d", &matrixSize, &wWidth, &wHeight,
           &bgColor.r, &bgColor.g, &bgColor.b, &lineColor.r, &lineColor.g, &lineColor.b);
    fclose(file);
}

void read_native() {
    std::cout << "Reading using native WinAPI" << std::endl;

    HANDLE hFile = CreateFile(FILENAME, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD bytesRead;
    char buf[256];

    if (ReadFile(hFile, buf, sizeof(buf), &bytesRead, nullptr)) {
        sscanf_s(buf, "%ld\n%ld\n%ld\n%d\n%d\n%d\n%d\n%d\n%d", &matrixSize, &wWidth, &wHeight,
                 &bgColor.r, &bgColor.g, &bgColor.b, &lineColor.r, &lineColor.g, &lineColor.b);
    }
    else {
        std::cerr << "Error reading file." << std::endl;
    }

    CloseHandle(hFile);
}

int main(int argc, char** argv) {
    BOOL bMessageOk;
    MSG message; /* Here message to the application are saved */
    WNDCLASS wincl = {0}; /* Data structure for the windowclass */

    /* Harcode show command num when use non-winapi entrypoint */
    int nCmdShow = SW_SHOW;
    /* Get handle */
    HINSTANCE hThisInstance = GetModuleHandle(nullptr);

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szWinClass;
    wincl.lpfnWndProc = WindowProcedure; /* This function is called by Windows */
    wincl.hCursor = LoadCursor(nullptr, IDC_ARROW); // https://stackoverflow.com/questions/72285491

    if (argc > 2 && atoi(argv[2]) >= 0) {
        dbMode = atoi(argv[2]);
        std::cout << "File configuration mode set to " << dbMode << std::endl;
        switch (dbMode) {
            case 1:
                read_memory();
                break;
            case 2:
                read_fopen();
                break;
            case 3:
                read_ifstream();
                break;
            case 4:
            default:
                read_native();
                break;
        }
    }
    else {
        read_native();
    }

    if (argc > 1 && atoi(argv[1]) > 0) {
        matrixSize = atoi(argv[1]);
    }

    if (matrixSize <= 0) {
        std::cout << "Field size must be greater than zero but its " << matrixSize << std::endl;
        return 0;
    }

    /* Use custom brush to paint the background of the window */
    hBrush = CreateSolidBrush(RGB(bgColor.r, bgColor.g, bgColor.b));
    wincl.hbrBackground = hBrush;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClass(&wincl))
        return 0;
    /* The class is registered, let's create the program*/
    hwnd = CreateWindow(
        szWinClass, /* Classname */
        szWinName, /* Title Text */
        WS_OVERLAPPEDWINDOW, /* default window */
        CW_USEDEFAULT, /* Windows decides the position */
        CW_USEDEFAULT, /* where the window ends up on the screen */
        wWidth, /* The programs width */
        wHeight, /* and height in pixels */
        HWND_DESKTOP, /* The window is a child-window to desktop */
        NULL, /* No menu */
        hThisInstance, /* Program Instance handler */
        NULL /* No Window Creation data */
    );

    HANDLE hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, matrixSize * matrixSize,
                                            szSharedMemoryName);
    matrix = (LPTSTR)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, matrixSize * matrixSize);

    // matrix = new int *[matrixSize];
    // for (int i = 0; i < matrixSize; i++) {
    //     matrix[i] = new int[matrixSize];
    //     for (int j = 0; j < matrixSize; j++) {
    //         matrix[i][j] = 0;
    //     }
    // }

    srand(time(nullptr));

    // synchMessage = RegisterWindowMessage("FieldUpdate");

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nCmdShow);
    /* Run the message loop. It will run until GetMessage() returns 0 */
    while ((bMessageOk = GetMessage(&message, nullptr, 0, 0)) != 0) {
        /* BOOL mb not only 1 or 0.
         * See msdn at https://msdn.microsoft.com/en-us/library/windows/desktop/ms644936(v=vs.85).aspx
         */
        if (bMessageOk == -1) {
            puts("Suddenly, GetMessage failed! You can call GetLastError() to see what happend");
            break;
        }
        /* Translate virtual-key message into character message */
        TranslateMessage(&message);
        /* Send message to WindowProcedure */
        DispatchMessage(&message);
    }

    /* Cleanup stuff */
    // for (int i = 0; i < matrixSize; i++) {
    //     delete[] matrix[i];
    // }
    // delete[] matrix;

    switch (dbMode) {
        case 1:
            write_memory();
            break;
        case 2:
            write_fopen();
            break;
        case 3:
            write_ifstream();
            break;
        case 4:
        default:
            write_native();
            break;
    }

    UnmapViewOfFile(matrix);
    CloseHandle(hFileMapping);

    DestroyWindow(hwnd);
    UnregisterClass(szWinClass, hThisInstance);
    DeleteObject(hBrush);

    return 0;
}
