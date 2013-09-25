// TestClient.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "TestClient.h"

#include <winsock2.h>
#include <windows.h>
#include <assert.h>


#include "..\..\PacketType.h"
#include "CircularBuffer.h"

#pragma comment(lib,"ws2_32.lib")

#define MAX_LOADSTRING 100

#define IDC_SEND_BUTTON	103
#define WM_SOCKET		104

#define BUFSIZE	1024*10


SOCKET g_Socket = NULL ;
CircularBuffer g_SendBuffer(BUFSIZE) ;
CircularBuffer g_RecvBuffer(BUFSIZE) ;

char* szServer = "localhost" ;
int nPort = 9001 ;


bool Initialize()
{
	WSADATA WsaDat ;

	int nResult = WSAStartup(MAKEWORD(2,2),&WsaDat) ;
	if ( nResult != 0 )
		return false ;

	g_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ;
	if ( g_Socket == INVALID_SOCKET )
		return false ;

	return true ;
}

bool Connect(const char* serverAddr, int port)
{
	// Resolve IP address for hostname
	struct hostent* host ;

	if ( (host=gethostbyname(serverAddr) ) == NULL )
		return false ;

	// Set up our socket address structure
	SOCKADDR_IN SockAddr ;
	SockAddr.sin_port = htons(port) ;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr) ;

	if ( SOCKET_ERROR == connect(g_Socket, (LPSOCKADDR)(&SockAddr), sizeof(SockAddr)) )
	{
		if (GetLastError() != WSAEWOULDBLOCK )
			return false ;
	}
	

	return true ;
}

/// ��Ŷó�� 
void ProcessPacket()
{
	while ( true )
	{
		PacketHeader header ;

		if ( false == g_RecvBuffer.Peek((char*)&header, sizeof(PacketHeader)) )
			break ;

		if (header.mSize > g_RecvBuffer.GetCurrentSize() )
			break ;

		switch ( header.mType )
		{
		case PKT_TEST:
			{
				TestEchoPacket recvData ;
				if ( g_RecvBuffer.Read((char*)&recvData, header.mSize) )
				{
					// ��Ŷó��
					recvData.mPlayerId ;
					recvData.mPosX ;
					recvData.mPosY ;
					recvData.mPosZ ;
					OutputDebugStringA(recvData.mData) ;
				}
				else
				{
					assert(false) ;
				}
			}
			break ;
		case PKT_SC_PONG:
			{
				TestPong recvData ;
				if ( g_RecvBuffer.Read((char*)&recvData, header.mSize) )
				{
					// ��Ŷó��
					recvData.mResult ;
					recvData.mPlayerId ;
					OutputDebugStringA(recvData.mData) ;
				}
				else
				{
					assert(false) ;
				}
			}
			break ;

		case PKT_SC_PONG2:
			{
				TestPong2 recvData ;
				if ( g_RecvBuffer.Read((char*)&recvData, header.mSize) )
				{
					// ��Ŷó��
					recvData.mResult ;
					recvData.mPlayerId ;
					recvData.mPosX ;
					recvData.mPosY ;
					recvData.mPosZ ;
				}
				else
				{
					assert(false) ;
				}
			}
			break ;
		default:
			assert(false) ;
		}

	}
}


// ���� ����:
HINSTANCE hInst;								// ���� �ν��Ͻ��Դϴ�.
TCHAR szTitle[MAX_LOADSTRING];					// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR szWindowClass[MAX_LOADSTRING];			// �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
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

 	// TODO: ���⿡ �ڵ带 �Է��մϴ�.
	MSG msg;
	HACCEL hAccelTable;


	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TESTCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTCLIENT));

	// �⺻ �޽��� �����Դϴ�.
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
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TESTCLIENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 100, 200, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����: �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND	- ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT	- �� â�� �׸��ϴ�.
//  WM_DESTROY	- ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{

		case WM_CREATE:
		{
			// Create a push button
			CreateWindow(L"BUTTON", L"Send", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
						10,	10, 75, 23, hWnd, (HMENU)IDC_SEND_BUTTON, GetModuleHandle(NULL), NULL);

			if ( false == Initialize() )
			{
				SendMessage(hWnd,WM_DESTROY,NULL,NULL) ;
				break ;
			}
			
			int	nResult=WSAAsyncSelect(g_Socket, hWnd, WM_SOCKET,(FD_CLOSE|FD_CONNECT));
			if (nResult)
			{
				MessageBox(hWnd, L"WSAAsyncSelect failed", L"Critical Error", MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			if ( false == Connect(szServer, nPort) )
			{
				SendMessage(hWnd,WM_DESTROY,NULL,NULL) ;
				break ;
			}
		
		}
		break;

	case WM_COMMAND:
		{
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// �޴� ������ ���� �м��մϴ�.

			switch (wmId)
			{
			case IDC_SEND_BUTTON:
			{
				static int pId = 1 ;

				TestEchoPacket sendData ;
				sendData.mPlayerId = pId++ ;
				sendData.mPosX = rand() ;
				sendData.mPosY = rand() ;
				sendData.mPosZ = rand() ;
				
				for ( int i=0 ; i <1024 ; ++i)
					sendData.mData[i] = 65 + i % 26 ;

				sendData.mData[1022] = '\n' ;
				sendData.mData[1023] = '\0' ;
				
				if ( g_SendBuffer.Write((const char*)&sendData, sendData.mSize) )
				{
					PostMessage(hWnd, WM_SOCKET, wParam, FD_WRITE) ;
				}
			}
			break;

			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;

		case WM_SOCKET:
		{
			if (WSAGETSELECTERROR(lParam))
			{	
				MessageBox(hWnd,L"WSAGETSELECTERROR",	L"Error", MB_OK|MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_CONNECT:
				{
					int nResult=WSAAsyncSelect(g_Socket, hWnd, WM_SOCKET, (FD_CLOSE|FD_READ|FD_WRITE) ) ;
					if (nResult)
					{
						assert(false) ;
						break;
					}
				}
				break ;

			case FD_READ:
				{
					char inBuf[4096] = {0, } ;
					
					int recvLen = recv(g_Socket, inBuf, 4096, 0) ;

					if ( !g_RecvBuffer.Write(inBuf, recvLen) )
					{
						/// ���� ��á��. 
						assert(false) ;
					}
					
					ProcessPacket() ;
					
				}
				break;

			case FD_WRITE:
				{
					/// ������ ���ۿ� �ִ°͵� ������ ������
					int size = g_SendBuffer.GetCurrentSize() ;
					if ( size > 0 )
					{
						char* data = new char[size] ;
						g_SendBuffer.Peek(data) ;

						int sent = send(g_Socket, data, size, 0) ;
						
						/// �ٸ��� �ִ�
						if ( sent != size )
							OutputDebugStringA("sent != request\n") ;

						g_SendBuffer.Consume(sent) ;

						delete [] data ;
					}
				
				}
				break ;

			case FD_CLOSE:
				{
					MessageBox(hWnd, L"Server closed connection", L"Connection closed!", MB_ICONINFORMATION|MB_OK);
					closesocket(g_Socket);
					SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				}
				break;
			}
		} 
		break ;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
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

