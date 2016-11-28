#include "stdafx.h"
#include "KeyRecPlay.h"
#include <vector>

#define MAX_LOADSTRING 100

#define BUTTON_SIZE_W 150             // ボタンの幅
#define BUTTON_SIZE_H  50             // ボタンの高さ

#define BUTTON_ID_REC  0              // REC ボタンがクリックされたことを識別するためのID
#define BUTTON_ID_PLAY 1              // PLAY ボタンがクリックされたことを識別するためのID

#define BUTTON_TEXT_REC  TEXT("REC")  // REC ボタンのテキスト
#define BUTTON_TEXT_PLAY TEXT("PLAY") // PLAY ボタンのテキスト
#define BUTTON_TEXT_STOP TEXT("STOP") // REC・PLAY ボタンの実行中のテキスト

#define EXTRA_INFO 12345              // KeyRecPlayで生成したキーイベントであることを識別するデータ

enum State {
	NONE, RECORDING, PLAYING
};

// グローバル変数:
HINSTANCE hInst;                      // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];        // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];  // メイン ウィンドウ クラス名

HWND hWnd;
HWND btnRec;
HWND btnPlay;
State state = NONE;

HHOOK hHookRec;
HHOOK hHookPlay;

std::vector<DWORD> keyCodes; // REC 中に入力されたキーコードを記憶する
int playingIndex;            // PLAY 中にキー入力があった際に keyCodes[playingIndex] のキーコードで置き換える

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// REC 中のキーボードフック
LRESULT CALLBACK recordingOnKeyPressed(int code, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_KEYDOWN) {
		// 押されたキーを記録
		LPKBDLLHOOKSTRUCT key = (LPKBDLLHOOKSTRUCT)lParam;
		keyCodes.push_back(key->vkCode);
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

// PLAY 中のキーボードフック
LRESULT CALLBACK playingOnKeyPressed(int code, WPARAM wParam, LPARAM lParam)
{
	LPKBDLLHOOKSTRUCT key = (LPKBDLLHOOKSTRUCT)lParam;

	// アプリケーションが生成したキーイベントなら通常通り処理する
	if (key->dwExtraInfo == EXTRA_INFO) {
		return CallNextHookEx(NULL, code, wParam, lParam);
	}

	if (wParam == WM_KEYDOWN) {

		DWORD overrideKeyCode = keyCodes[playingIndex];

		// キーイベントを送る
		keybd_event(overrideKeyCode, 0, 0, EXTRA_INFO);

		// 次の記録位置へ
		playingIndex++;

		// 末尾に到達したら最初に戻す
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

	// TODO: ここにコードを挿入してください。

	// グローバル文字列を初期化しています。
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_KEYRECPLAY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYRECPLAY));

	MSG msg;

	// メイン メッセージ ループ:
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
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
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
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

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
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウの描画
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 選択されたメニューの解析:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

			// RECボタンがクリックされた時
		case BUTTON_ID_REC:

			// 何もしていない状態なら、レコーディング状態にする
			if (state == NONE) {
				state = RECORDING;

				// UI を更新
				SetWindowText(btnRec, BUTTON_TEXT_STOP);
				EnableWindow(btnPlay, FALSE);

				// グローバルキーボードフックを追加
				hHookRec = SetWindowsHookEx(WH_KEYBOARD_LL, recordingOnKeyPressed, hInst, 0);

				// 前回記録したキーコードを初期化
				keyCodes.clear();
			}
			// レコーディング状態なら、何もしていない状態にする
			else {
				state = NONE;
				SetWindowText(btnRec, BUTTON_TEXT_REC);
				EnableWindow(btnPlay, TRUE);

				// グローバルキーボードフックを解除
				UnhookWindowsHookEx(hHookRec);
			}
			break;

			// PLAYボタンがクリックされた時
		case BUTTON_ID_PLAY:

			// 何もしていない状態なら、再生状態にする
			if (state == NONE) {

				// 1個以上キーを記録している場合のみ
				if (!keyCodes.empty()) {

					state = PLAYING;

					// UI を更新
					SetWindowText(btnPlay, BUTTON_TEXT_STOP);
					EnableWindow(btnRec, FALSE);

					// グローバルキーボードフックを追加
					hHookPlay = SetWindowsHookEx(WH_KEYBOARD_LL, playingOnKeyPressed, hInst, 0);

					// 再生位置を初期化
					playingIndex = 0;
				}
			}
			// レコーディング状態なら、何もしていない状態にする
			else {
				state = NONE;

				// UI を更新
				SetWindowText(btnPlay, BUTTON_TEXT_PLAY);
				EnableWindow(btnRec, TRUE);

				// グローバルキーボードフックを解除
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
