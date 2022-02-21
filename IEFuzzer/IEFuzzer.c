#include <windows.h>
#include "cwebpage.h"
#include <stdio.h>



// A running count of how many windows we have open that contain a browser object
unsigned char WindowCount = 0;

// The class name of our Window to host the browser. It can be anything of your choosing.
static const TCHAR	ClassName[] = "Browser Example";

// Where we store the pointers to CWebPage.dll's functions
EmbedBrowserObjectPtr		*lpEmbedBrowserObject;
UnEmbedBrowserObjectPtr		*lpUnEmbedBrowserObject;
DisplayHTMLPagePtr			*lpDisplayHTMLPage;
DisplayHTMLStrPtr			*lpDisplayHTMLStr;
ResizeBrowserPtr			*lpResizeBrowser;





/****************************** WindowProc() ***************************
 * Our message handler for our window to host the browser.
 */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
		{
			// Resize the browser object to fit the window
			(*lpResizeBrowser)(hwnd, LOWORD(lParam), HIWORD(lParam));
			return(0);
		}

		case WM_CREATE:
		{
			// Embed the browser object into our host window. We need do this only
			// once. Note that the browser object will start calling some of our
			// IOleInPlaceFrame and IOleClientSite functions as soon as we start
			// calling browser object functions in EmbedBrowserObject().
			if ((*lpEmbedBrowserObject)(hwnd)) return(-1);
			// Another window created with an embedded browser object
			++WindowCount;
			printf("WindowCount: %x\n", WindowCount);
			return(0);
		}

		case WM_DESTROY:
		{
			// Detach the browser object from this window, and free resources.
			(*lpUnEmbedBrowserObject)(hwnd);
			// One less window
			--WindowCount;
			printf("WindowCount: %x\n", WindowCount);
			// If all the windows are now closed, quit this app
			if (!WindowCount) PostQuitMessage(0);
			return(TRUE);
		}


	}
	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}

int main(int argc, char **argv)
{
	HINSTANCE		cwebdll;
	MSG				msg;
	WNDCLASSEX		wc;

	if (argc != 2) {
		printf("exe_file path_to_file");
		return 0;
	}

	// Load our DLL containing the OLE/COM code. We do this once-only. It's named "cwebpage.dll"
	if ((cwebdll = LoadLibrary("cwebpage.dll")))
	{
		// Get pointers to the EmbedBrowserObject, DisplayHTMLPage, DisplayHTMLStr, and UnEmbedBrowserObject
		// functions, and store them in some globals.
		lpEmbedBrowserObject	= (EmbedBrowserObjectPtr *)		GetProcAddress((HINSTANCE)cwebdll, EMBEDBROWSEROBJECTNAME);
		lpUnEmbedBrowserObject	= (UnEmbedBrowserObjectPtr *)	GetProcAddress((HINSTANCE)cwebdll, UNEMBEDBROWSEROBJECTNAME);
		lpDisplayHTMLPage		= (DisplayHTMLStrPtr *)			GetProcAddress((HINSTANCE)cwebdll, DISPLAYHTMLPAGENAME);
		lpDisplayHTMLStr		= (DisplayHTMLStrPtr *)			GetProcAddress((HINSTANCE)cwebdll, DISPLAYHTMLSTRNAME);
		lpResizeBrowser			= (ResizeBrowserPtr *)			GetProcAddress((HINSTANCE)cwebdll, RESIZEBROWSERNAME);

		// Register the class of our window to host the browser. 'WindowProc' is our message handler
		// and 'ClassName' is the class name. You can choose any class name you want.
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		wc.cbSize			= sizeof(WNDCLASSEX);
		wc.hInstance		= GetModuleHandle(NULL);
		wc.lpfnWndProc		= WindowProc;
		wc.lpszClassName	= &ClassName[0];
		RegisterClassEx(&wc);

		// Create another window with another browser object embedded in it.
		if ((msg.hwnd = CreateWindowEx(0, &ClassName[0], "page", WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
						HWND_DESKTOP, NULL, GetModuleHandle(NULL), 0)))
		{
			// For this window, display a URL. This could also be a HTML file on disk such as "c:\\myfile.htm".
			(*lpDisplayHTMLPage)(msg.hwnd, argv[1]);

			// Show the window.
			ShowWindow(msg.hwnd, SW_SHOW);
			UpdateWindow(msg.hwnd);
		}

		// Do a message loop until WM_QUIT.
		while (GetMessage(&msg, NULL, 0, 0)) {

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return(0);
	}
	MessageBox(0, "Can't open cwebpage.dll!", "ERROR", MB_OK);
	return(-1);
}
