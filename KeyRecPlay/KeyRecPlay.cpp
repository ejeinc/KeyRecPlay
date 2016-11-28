#include "stdafx.h"
#include "KeyRecPlay.h"
#include <vector>

#define MAX_LOADSTRING 100

#define BUTTON_SIZE_W 150             // �{�^���̕�
#define BUTTON_SIZE_H  50             // �{�^���̍���

#define BUTTON_ID_REC  0              // REC �{�^�����N���b�N���ꂽ���Ƃ����ʂ��邽�߂�ID
#define BUTTON_ID_PLAY 1              // PLAY �{�^�����N���b�N���ꂽ���Ƃ����ʂ��邽�߂�ID

#define BUTTON_TEXT_REC  TEXT("REC")  // REC �{�^���̃e�L�X�g
#define BUTTON_TEXT_PLAY TEXT("PLAY") // PLAY �{�^���̃e�L�X�g
#define BUTTON_TEXT_STOP TEXT("STOP") // REC�EPLAY �{�^���̎��s���̃e�L�X�g

#define EXTRA_INFO 12345              // KeyRecPlay�Ő��������L�[�C�x���g�ł��邱�Ƃ����ʂ���f�[�^

enum State {
	NONE, RECORDING, PLAYING
};

// �O���[�o���ϐ�:
HINSTANCE hInst;                      // ���݂̃C���^�[�t�F�C�X
WCHAR szTitle[MAX_LOADSTRING];        // �^�C�g�� �o�[�̃e�L�X�g
WCHAR szWindowClass[MAX_LOADSTRING];  // ���C�� �E�B���h�E �N���X��

HWND hWnd;
HWND btnRec;
HWND btnPlay;
State state = NONE;

HHOOK hHookRec;
HHOOK hHookPlay;

std::vector<DWORD> keyCodes; // REC ���ɓ��͂��ꂽ�L�[�R�[�h���L������
int playingIndex;            // PLAY ���ɃL�[���͂��������ۂ� keyCodes[playingIndex] �̃L�[�R�[�h�Œu��������

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂�:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// REC ���̃L�[�{�[�h�t�b�N
LRESULT CALLBACK recordingOnKeyPressed(int code, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_KEYDOWN) {
		// �����ꂽ�L�[���L�^
		LPKBDLLHOOKSTRUCT key = (LPKBDLLHOOKSTRUCT)lParam;
		keyCodes.push_back(key->vkCode);
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

// PLAY ���̃L�[�{�[�h�t�b�N
LRESULT CALLBACK playingOnKeyPressed(int code, WPARAM wParam, LPARAM lParam)
{
	LPKBDLLHOOKSTRUCT key = (LPKBDLLHOOKSTRUCT)lParam;

	// �A�v���P�[�V���������������L�[�C�x���g�Ȃ�ʏ�ʂ菈������
	if (key->dwExtraInfo == EXTRA_INFO) {
		return CallNextHookEx(NULL, code, wParam, lParam);
	}

	if (wParam == WM_KEYDOWN) {

		DWORD overrideKeyCode = keyCodes[playingIndex];

		// �L�[�C�x���g�𑗂�
		keybd_event(overrideKeyCode, 0, 0, EXTRA_INFO);

		// ���̋L�^�ʒu��
		playingIndex++;

		// �����ɓ��B������ŏ��ɖ߂�
		if (playingIndex >= keyCodes.size()) {
			playingIndex = 0;
		}
	}

	return 1;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: �����ɃR�[�h��}�����Ă��������B

	// �O���[�o������������������Ă��܂��B
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_KEYRECPLAY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYRECPLAY));

	MSG msg;

	// ���C�� ���b�Z�[�W ���[�v:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//
//  �֐�: MyRegisterClass()
//
//  �ړI: �E�B���h�E �N���X��o�^���܂��B
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYRECPLAY));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_KEYRECPLAY);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   �֐�: InitInstance(HINSTANCE, int)
//
//   �ړI: �C���X�^���X �n���h����ۑ����āA���C�� �E�B���h�E���쐬���܂��B
//
//   �R�����g:
//
//        ���̊֐��ŁA�O���[�o���ϐ��ŃC���X�^���X �n���h����ۑ����A
//        ���C�� �v���O���� �E�B���h�E���쐬����ѕ\�����܂��B
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		BUTTON_SIZE_W * 2 + 16, BUTTON_SIZE_H + 40,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	btnRec = CreateWindow(TEXT("BUTTON"), TEXT("REC"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 0,
		BUTTON_SIZE_W, BUTTON_SIZE_H,
		hWnd, (HMENU)BUTTON_ID_REC, hInstance, nullptr);

	btnPlay = CreateWindow(TEXT("BUTTON"), TEXT("PLAY"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		BUTTON_SIZE_W, 0,
		BUTTON_SIZE_W, BUTTON_SIZE_H,
		hWnd, (HMENU)BUTTON_ID_PLAY, hInstance, nullptr);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  �֐�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  �ړI:    ���C�� �E�B���h�E�̃��b�Z�[�W���������܂��B
//
//  WM_COMMAND  - �A�v���P�[�V���� ���j���[�̏���
//  WM_PAINT    - ���C�� �E�B���h�E�̕`��
//  WM_DESTROY  - ���~���b�Z�[�W��\�����Ė߂�
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// �I�����ꂽ���j���[�̉��:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

			// REC�{�^�����N���b�N���ꂽ��
		case BUTTON_ID_REC:

			// �������Ă��Ȃ���ԂȂ�A���R�[�f�B���O��Ԃɂ���
			if (state == NONE) {
				state = RECORDING;

				// UI ���X�V
				SetWindowText(btnRec, BUTTON_TEXT_STOP);
				EnableWindow(btnPlay, FALSE);

				// �O���[�o���L�[�{�[�h�t�b�N��ǉ�
				hHookRec = SetWindowsHookEx(WH_KEYBOARD_LL, recordingOnKeyPressed, hInst, 0);

				// �O��L�^�����L�[�R�[�h��������
				keyCodes.clear();
			}
			// ���R�[�f�B���O��ԂȂ�A�������Ă��Ȃ���Ԃɂ���
			else {
				state = NONE;
				SetWindowText(btnRec, BUTTON_TEXT_REC);
				EnableWindow(btnPlay, TRUE);

				// �O���[�o���L�[�{�[�h�t�b�N������
				UnhookWindowsHookEx(hHookRec);
			}
			break;

			// PLAY�{�^�����N���b�N���ꂽ��
		case BUTTON_ID_PLAY:

			// �������Ă��Ȃ���ԂȂ�A�Đ���Ԃɂ���
			if (state == NONE) {

				// 1�ȏ�L�[���L�^���Ă���ꍇ�̂�
				if (!keyCodes.empty()) {

					state = PLAYING;

					// UI ���X�V
					SetWindowText(btnPlay, BUTTON_TEXT_STOP);
					EnableWindow(btnRec, FALSE);

					// �O���[�o���L�[�{�[�h�t�b�N��ǉ�
					hHookPlay = SetWindowsHookEx(WH_KEYBOARD_LL, playingOnKeyPressed, hInst, 0);

					// �Đ��ʒu��������
					playingIndex = 0;
				}
			}
			// ���R�[�f�B���O��ԂȂ�A�������Ă��Ȃ���Ԃɂ���
			else {
				state = NONE;

				// UI ���X�V
				SetWindowText(btnPlay, BUTTON_TEXT_PLAY);
				EnableWindow(btnRec, TRUE);

				// �O���[�o���L�[�{�[�h�t�b�N������
				UnhookWindowsHookEx(hHookPlay);
			}
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
