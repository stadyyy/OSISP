#include <windows.h>
#include "resource.h"
#include <Richedit.h>

HINSTANCE hInst; // ���������� �������� ���������� ����������
HWND hMainWnd;   // ���������� �������� ����
HWND hEdit;      // ���������� �������� ���������� EDIT (��������� ����)
HMENU hMenu;     // ���������� ���� ����������

static WCHAR szFileName[MAX_PATH] = L""; // ��������� ��������� ��� �����


// ��������� �������
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TextEditor", nullptr };
    RegisterClassEx(&wcex);

    hInst = hInstance;
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    hMainWnd = CreateWindow(L"TextEditor", L"Text Editor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

    if (!hMainWnd)
    {
        return FALSE;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);
    
    RegisterHotKey(hMainWnd, HOTKEY_CTRL_S, MOD_CONTROL, 'S');
    RegisterHotKey(hMainWnd, HOTKEY_CTRL_O, MOD_CONTROL, 'O');
    RegisterHotKey(hMainWnd, HOTKEY_CTRL_SHIFT_S, MOD_CONTROL | MOD_SHIFT, 'S');

     
    // �������� ����
    hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    HMENU hEditMenu = CreateMenu();

    // ��������� ������ ���� ��� ���� "File"
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_NEW, L"New");
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"Open");
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_SAVE, L"Save");
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_SAVEAS, L"Save As");

    // ��������� ������ ���� ��� ���� "Edit"
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, L"Edit");
    AppendMenu(hEditMenu, MF_STRING, IDM_EDIT_COPY, L"Copy");
    AppendMenu(hEditMenu, MF_STRING, IDM_EDIT_PASTE, L"Paste");
    AppendMenu(hEditMenu, MF_STRING, IDM_EDIT_CUT, L"Cut");
    AppendMenu(hEditMenu, MF_STRING, IDM_EDIT_SELECTALL, L"Select All");

    // ���� "Styles"
    HMENU hStylesMenu = CreateMenu();
    AppendMenu(hStylesMenu, MF_STRING, IDM_CHANGE_BG, L"Background Color");
    AppendMenu(hStylesMenu, MF_STRING, IDM_SETFONT, L"Font Styles");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hStylesMenu, L"Styles");

    // ��������� ���� "Help" � ������� ����
    AppendMenu(hMenu, MF_STRING, IDM_HELP_ABOUT, L"Help");

    // ������������� ���� � ������� ����
    SetMenu(hMainWnd, hMenu);

    // �������� ���� ���������
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

// ���������� ��������� ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {

    case WM_CREATE:
    {
        LoadLibrary(TEXT("Msftedit.dll"));
        hEdit = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            MSFTEDIT_CLASS,
            NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            0, 0, 800, 600,
            hWnd,
            (HMENU)IDC_TEXT_EDIT,
            hInst,
            NULL);

        if (hEdit == NULL)
        {
            MessageBox(NULL, L"Cannot create edit control.", L"Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        // ������������� ���� � ������� ����
        SetMenu(hWnd, hMenu); 
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        if (wmEvent == 0) // ���������, ��� ��� ��������� �� ����
        {
            // ��������� ��������� ����
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_FILE_NEW:
                // ��������� ������ ���� "New"
                SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)L""); // ������� ��������� ����
                break;
            case IDM_FILE_OPEN:
            {
                OPENFILENAME ofn;

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hMainWnd;
                ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.lpstrFile = szFileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST;

                if (GetOpenFileName(&ofn))
                {
                    // ��������� ��������� ���� ��� ������
                    HANDLE hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        DWORD dwFileSize = GetFileSize(hFile, NULL);
                        if (dwFileSize != INVALID_FILE_SIZE)
                        {
                            // ��������� ������ ��� ������ �����
                            wchar_t* pBuffer = new wchar_t[dwFileSize / sizeof(wchar_t) + 1];
                            if (pBuffer)
                            {
                                DWORD dwBytesRead;
                                if (ReadFile(hFile, pBuffer, dwFileSize, &dwBytesRead, NULL))
                                {
                                    pBuffer[dwBytesRead / sizeof(wchar_t)] = L'\0'; // ��������� ������� ������ � �����, ����� ������� ������
                                    SetWindowTextW(hEdit, pBuffer); // ������������� ���������� ����� � ��������� ����
                                }

                                delete[] pBuffer;
                            }
                        }

                        CloseHandle(hFile);
                    }
                    // ��������� ��� �����
                    wcscpy_s(szFileName, MAX_PATH, ofn.lpstrFile);

                    // ������������� �������� ����� � ��������� ����
                    SetWindowTextW(hMainWnd, szFileName);
                }
            }
            break;
            case IDM_FILE_SAVE:
                // ��������� ������ ���� "Save"
                if (szFileName[0] == L'\0') // ���� � ��� ��� ����� ����� (���� �� ��� ������ ��� �������� �����)
                {
                    // ���� ��� ����� �����, �������� ������� "Save As"
                    SendMessage(hMainWnd, WM_COMMAND, IDM_FILE_SAVEAS, 0);
                }
                else // ���� � ��� ��� ���� ��� �����
                {
                    // ��������� ��������� ���� ��� ������
                    HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        int textLength = GetWindowTextLength(hEdit);
                        wchar_t* buffer = new wchar_t[textLength + 1];
                        GetWindowText(hEdit, buffer, textLength + 1);

                        DWORD dwBytesWritten;
                        WriteFile(hFile, buffer, textLength * sizeof(wchar_t), &dwBytesWritten, NULL);

                        delete[] buffer;
                        CloseHandle(hFile);
                    }
                }
                break;

            case IDM_FILE_SAVEAS:
            {
                OPENFILENAME ofn;

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hMainWnd;
                ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.lpstrFile = szFileName; // ���������� ���������� ��� �����
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_OVERWRITEPROMPT;

                if (GetSaveFileName(&ofn))
                {
                    int textLength = GetWindowTextLength(hEdit);
                    wchar_t* buffer = new wchar_t[textLength + 1];
                    GetWindowText(hEdit, buffer, textLength + 1);

                    // ��������� ���������� .txt, ���� ��� ���
                    if (wcsstr(szFileName, L".") == NULL)
                    {
                        wcscat_s(szFileName, MAX_PATH, L".txt");
                    }

                    // ��������� ��������� ���� ��� ������
                    HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        DWORD dwBytesWritten;
                        WriteFile(hFile, buffer, textLength * sizeof(wchar_t), &dwBytesWritten, NULL);
                        CloseHandle(hFile);
                    }

                    delete[] buffer;
                }
            }
            break;

            case IDM_EDIT_COPY:
                SendMessage(hEdit, WM_COPY, 0, 0);
                break;

            case IDM_EDIT_PASTE:
                SendMessage(hEdit, WM_PASTE, 0, 0);
                break;
            case IDM_EDIT_CUT:
                SendMessage(hEdit, WM_CUT, 0, 0);
                break;
            case IDM_EDIT_SELECTALL:
                SendMessage(hEdit, EM_SETSEL, 0, -1);
                break;


            case IDM_CHANGE_BG:
            {
                // ���������� ������ ������ �����
                CHOOSECOLOR cc;
                static COLORREF customColors[16];

                ZeroMemory(&cc, sizeof(CHOOSECOLOR));
                cc.lStructSize = sizeof(CHOOSECOLOR);
                cc.hwndOwner = hWnd;
                cc.lpCustColors = customColors;
                cc.rgbResult = RGB(255, 255, 255);  // ��������� ���� ���� (�����)
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;

                if (ChooseColor(&cc))
                {
                    // ������������� ���� ���� � �������� ���������� Rich Edit
                    SendMessage(hEdit, EM_SETBKGNDCOLOR, 0, cc.rgbResult);
                }
            }
            break;
            case IDM_SETFONT:
            {
                CHOOSEFONT cf;
                LOGFONT lf;

                // ������������� ��������� CHOOSEFONT
                ZeroMemory(&cf, sizeof(CHOOSEFONT));
                cf.lStructSize = sizeof(CHOOSEFONT);
                cf.hwndOwner = hWnd;
                cf.lpLogFont = &lf;
                cf.rgbColors = RGB(0, 0, 0);  // ��������� ���� ������ (������)
                cf.Flags = CF_EFFECTS | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_NOFACESEL;

                // ������������� ��������� LOGFONT
                ZeroMemory(&lf, sizeof(LOGFONT));
                lf.lfHeight = 12;  // ��������� ������ ������
                wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Arial");  // ��������� ��� ������

                // ����������� ������� ������ ������ � �����
                if (ChooseFont(&cf))
                {
                    // �������� ������ ������ �� ������ ��������� ����������
                    HFONT hFont = CreateFontIndirect(&lf);

                    if (hFont)
                    {
                        // ������������� ����� ����� � �������� ���������� Rich Edit
                        SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

                        // ������� CHARFORMAT2 ��� ��������� ����� ������
                        CHARFORMAT2 cf2;
                        memset(&cf2, 0, sizeof(CHARFORMAT2));
                        cf2.cbSize = sizeof(CHARFORMAT2);
                        cf2.dwMask = CFM_COLOR;
                        cf2.crTextColor = cf.rgbColors;  // ������������� ��������� ����

                        // ������������� ���� ������ � �������� ����������
                        SendMessage(hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2);
                    }
                }
            }
            break;


            case IDM_HELP_ABOUT:
                // ��������� ������ ���� "About"
                MessageBox(hWnd, L"Text Editor v1.0\n\n� by Stas", L"About", MB_ICONINFORMATION | MB_OK);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }

        }



        else if (wmId == IDC_TEXT_EDIT && wmEvent == EN_UPDATE)
        {
            // ��������� ��������� � ��������� ����
            int textLength = GetWindowTextLength(hEdit);
            wchar_t* buffer = new wchar_t[textLength + 1];
            GetWindowText(hEdit, buffer, textLength + 1);
            // ������ ���������� 'buffer' �������� ����� �� ���������� ����
            delete[] buffer;
        }
        else
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_HOTKEY:
    {
        int id = wParam;
        switch (id)
        {
        case HOTKEY_CTRL_O:
            SendMessage(hWnd, WM_COMMAND, IDM_FILE_OPEN, 0);
            break;
        case HOTKEY_CTRL_S:
            SendMessage(hWnd, WM_COMMAND, IDM_FILE_SAVE, 0);
            break;
        case HOTKEY_CTRL_SHIFT_S:
            SendMessage(hWnd, WM_COMMAND, IDM_FILE_SAVEAS, 0);
            break;
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);  //��� ��������� ����
       

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_SIZE:
        // TODO: ��������� ��������� ������� ����
        break;
    case WM_DESTROY:
        PostQuitMessage(0); //���������� ��������� ����� ��������� � �������� ����������
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}