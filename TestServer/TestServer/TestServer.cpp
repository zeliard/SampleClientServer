// TestServer.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "TestServer.h"

#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>

#include "..\..\PacketType.h"
#include "ABCircularBuffer.h"

#pragma comment(lib,"ws2_32.lib")

#define MAX_LOADSTRING 100

#define BUF_SIZE	(1024*10)
#define	WM_SOCKET	(WM_USER+1)


// 일단 테스트용 에코 
struct ClientInfo
{
	ClientInfo() : mClientId(-1), mSock(NULL), mEchoBuffer(BUF_SIZE)
	{}
	int		mClientId ;
	SOCKET	mSock ;

	ABCircularBuffer	mEchoBuffer ;

} ;

typedef std::map<SOCKET, ClientInfo*> ClientMap ;
ClientMap g_ClientMap ;

int g_ClientIndex = 0 ; ///< overflow 인정

void AddClient(SOCKET sock) ;
void DelClient(SOCKET sock) ;
ClientInfo* GetClientInfo(SOCKET sock) ;

// 소켓 관련 윈도우 메시지 처리
void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;


// 전역 변수:
HINSTANCE hInst;								// 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];					// 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];			// 기본 창 클래스 이름입니다.

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 여기에 코드를 입력합니다.
	MSG msg;
	HACCEL hAccelTable;

	// 전역 문자열을 초기화합니다.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TESTSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}


	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTSERVER));

	// 기본 메시지 루프입니다.
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTSERVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TESTSERVER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);


   /// winsock 
   WSADATA wsa ;
   if ( WSAStartup(MAKEWORD(2,2), &wsa) != 0 )
	   return FALSE ;

   SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0) ;
   if ( listenSock == INVALID_SOCKET )
	   return FALSE ;

   int retVal = WSAAsyncSelect(listenSock, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE  ) ;
   if ( retVal == SOCKET_ERROR )
	   return FALSE ;

   // bind()
   SOCKADDR_IN serveraddr ;
   ZeroMemory(&serveraddr, sizeof(serveraddr)) ;
   serveraddr.sin_family = AF_INET ;
   serveraddr.sin_port = htons(9001) ;
   serveraddr.sin_addr.s_addr = htonl(INADDR_ANY) ;
   retVal = bind(listenSock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) ;
   if ( retVal == SOCKET_ERROR )
	   return FALSE ;

   // listen()
   retVal = listen(listenSock, SOMAXCONN) ;
   if ( retVal == SOCKET_ERROR )
	   return FALSE ;


   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND	- 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT	- 주 창을 그립니다.
//  WM_DESTROY	- 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 메뉴 선택을 구문 분석합니다.
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 그리기 코드를 추가합니다.
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SOCKET:
		ProcessSocketMessage(hWnd, message, wParam, lParam) ;
		break ;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


////////////////////////////////////////

void AddClient(SOCKET sock)
{
	ClientInfo* newClient = new ClientInfo ;

	newClient->mClientId = ++g_ClientIndex ;
	newClient->mSock = sock ;

	g_ClientMap.insert(ClientMap::value_type(sock, newClient)) ;
}

void DelClient(SOCKET sock)
{
	ClientMap::iterator it = g_ClientMap.find(sock) ;
	if ( it != g_ClientMap.end() )
	{
		ClientInfo* delClient = it->second ;
		delete delClient ;
		g_ClientMap.erase(it) ;
	}
}

ClientInfo* GetClientInfo(SOCKET sock)
{
	ClientMap::iterator it = g_ClientMap.find(sock) ;
	if ( it != g_ClientMap.end() )
	{
		ClientInfo* client = it->second ;
		return client ;
	}

	return nullptr ;
}


void err_print(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER| FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL) ;

	MessageBoxA(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}


void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 데이터 통신에 사용할 변수

	SOCKADDR_IN clientaddr ;
	int addrlen = sizeof(clientaddr) ;
	int retval ;

	// 오류 발생 여부 확인
	if ( WSAGETSELECTERROR(lParam) )
	{
		DelClient(wParam) ;
		return ;
	}

	// 메시지 처리
	switch ( WSAGETSELECTEVENT(lParam) )
	{
	case FD_ACCEPT:
		{
			SOCKET client_sock = accept(wParam, (SOCKADDR *)&clientaddr, &addrlen) ;
			if ( client_sock == INVALID_SOCKET )
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
					err_print("accept()") ;
				return ;
			}

		
			AddClient(client_sock) ;
			retval = WSAAsyncSelect(client_sock, hWnd, WM_SOCKET, FD_READ|FD_WRITE|FD_CLOSE) ;
			if (retval == SOCKET_ERROR)
			{
				err_print("WSAAsyncSelect()") ;
				DelClient(client_sock) ;
			}
		}
		break;

	case FD_READ:
		{
			ClientInfo* client = GetClientInfo(wParam) ;
			
			int space = 0 ;
			char* pData = client->mEchoBuffer.Reserve(BUF_SIZE, space) ;
			if (pData == NULL)
			{
				err_print("Reserve Error") ;
				return ;
			}

			int recvcount = recv(client->mSock, (char*)pData, space, 0) ;
			if (recvcount == SOCKET_ERROR)
			{
				if(WSAGetLastError() != WSAEWOULDBLOCK)
				{
					err_print("recv()") ;
					DelClient(wParam) ;
				}
				return ;
			}
			
			client->mEchoBuffer.Commit(recvcount) ;
			

			/// echo send request
			PostMessage(hWnd, WM_SOCKET, client->mSock, FD_WRITE) ;
			
			
		}
		break ;
	
		
	case FD_WRITE:
		{
			ClientInfo* client = GetClientInfo(wParam) ;

			int allocated = 0 ;
			char* pData = nullptr ;

			while ( (pData = client->mEchoBuffer.GetFirstDataBlock(allocated)) != nullptr )
			{
				int sendcount = send(client->mSock, pData, allocated, 0) ;

				if (sendcount == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK )
					{
						err_print("send()") ;
						DelClient(wParam) ;
					}
					return ;
				}
	
				client->mEchoBuffer.DecommitFirstDataBlock(sendcount) ;
			}

		}
		break ;

	case FD_CLOSE:
		DelClient(wParam) ;
		break;
	}
}
