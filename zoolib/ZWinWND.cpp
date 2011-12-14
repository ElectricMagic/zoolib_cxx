/* -------------------------------------------------------------------------------------------------
Copyright (c) 2011 Andrew Green
http://www.zoolib.org

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------------------------- */

#include "zoolib/ZWinWND.h"

#if ZCONFIG_SPI_Enabled(Win)

#include "zoolib/ZDebug.h"
#include "zoolib/ZLog.h"
#include "zoolib/ZUtil_Win.h"

namespace ZooLib {
namespace ZWinWND {

// =================================================================================================
#pragma mark -
#pragma mark * Helpers (anonymous)

namespace { // anonymous

ZRef<Callable> spGetCallable(HWND iHWND)
	{
	DWORD windowProcessID;
	::GetWindowThreadProcessId(iHWND, &windowProcessID);

	if (windowProcessID == ::GetCurrentProcessId())
		return (Callable*)::GetPropW(iHWND, L"WinWND Callable");

	return null;
	}

WNDPROC spGetBasePROC(HWND iHWND)
	{ return (WNDPROC)::GetPropW(iHWND, L"ZWinWND BasePROC"); }

void spAttach(HWND iHWND, WNDPROC basePROC, ZRef<Callable> iCallable)
	{
	::SetPropW(iHWND, L"ZWinWND BasePROC", basePROC);

	iCallable->Retain();
	::SetPropW(iHWND, L"WinWND Callable", iCallable.Get());
	}

struct CreateStruct
	{
	WNDPROC fWNDPROC;
	ZRef<Callable> fCallable;
	};

LRESULT CALLBACK spWindowProcW(HWND iHWND, UINT iMessage, WPARAM iWPARAM, LPARAM iLPARAM)
	{
	if (WNDPROC baseProc = spGetBasePROC(iHWND))
		{
		if (ZRef<Callable> theCallable = spGetCallable(iHWND))
			{
			if (iMessage == WM_NCDESTROY)
				{
				// Undo the Retain we did when we attached.
				theCallable->Release();
				}
			return theCallable->Call(baseProc, iHWND, iMessage, iWPARAM, iLPARAM);
			}
		else
			{
			// Until we allow a callable to be detached we'll never get here.
			return ::CallWindowProcW(baseProc, iHWND, iMessage, iWPARAM, iLPARAM);
			}
		}
	else if (iMessage == WM_GETMINMAXINFO)
		{
		// We're being created, but the *very* first message sent to a window
		// is (go figure) WM_GETMINMAXINFO. We don't have access to our CREATESTRUCTW,
		// so we'll handle it specially.
		return ::DefWindowProcW(iHWND, iMessage, iWPARAM, iLPARAM);
		}
	else
		{
		ZAssert(iMessage == WM_NCCREATE);
		// This is the second message sent to a window, and is where we're able to
		// attach to our Callable. All subsequent calls will go through the callable.
		CreateStruct* theCS = (CreateStruct*)((CREATESTRUCTW*)iLPARAM)->lpCreateParams;
		spAttach(iHWND, theCS->fWNDPROC, theCS->fCallable);
		return theCS->fCallable->Call(theCS->fWNDPROC, iHWND, WM_NCCREATE, iWPARAM, iLPARAM);
		}
	}

INT_PTR CALLBACK spDialogProcW(HWND iHWND, UINT iMessage, WPARAM iWPARAM, LPARAM iLPARAM)
	{
	if (ZRef<Callable_Dialog> theCallable =
		reinterpret_cast<Callable_Dialog*>((long long)::GetWindowLongPtrW(iHWND, GWLP_USERDATA)))
		{
		if (iMessage == WM_NCDESTROY)
			{
			// Undo the Retain we did in WM_INITDIALOG.
			theCallable->Release();
			}
		return theCallable->Call(iHWND, iMessage, iWPARAM, iLPARAM);
		}
	else if (iMessage == WM_INITDIALOG)
		{
		::SetWindowLongPtrW(iHWND, GWLP_USERDATA, iLPARAM);
		theCallable = (Callable_Dialog*)iLPARAM;
		theCallable->Retain();
		return theCallable->Call(iHWND, iMessage, iWPARAM, iLPARAM);
		}
	else
		{
		return false;
		}
	}
	
} // anonymous namespace

// =================================================================================================
#pragma mark -
#pragma mark * ZWinWND::ClassRegistration

ClassRegistration::ClassRegistration(WNDPROC iWNDPROC, const WCHAR* iClassName)
:	fClassName(iClassName)
	{
	WNDCLASSW windowClass;
	windowClass.style = 0;
	windowClass.lpfnWndProc = iWNDPROC;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = ZUtil_Win::sGetModuleHandle();
	windowClass.hIcon = nullptr;
	windowClass.hCursor = ::LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW));
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = iClassName;
	fATOM = ::RegisterClassW(&windowClass);
	ZAssertStop(0, fATOM != 0);
	}

ClassRegistration::~ClassRegistration()
	{
	if (not ::UnregisterClassW(fClassName, ZUtil_Win::sGetModuleHandle()))
		{
		if (ZLOGPF(s, eInfo))
			s << "Failed for << " << fClassName;
		}
	}

const WCHAR* ClassRegistration::GetClassName() const
	{ return fClassName; }

// =================================================================================================
#pragma mark -
#pragma mark * ZWinWND, Callable <--> Regular window

static ClassRegistration spClassRegistration(spWindowProcW, L"ZWinWND ClassRegistration");

HWND sCreate
	(DWORD dwExStyle,
	LPCWSTR lpWindowName,
	DWORD dwStyle,
	int X,
	int Y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	WNDPROC iWNDPROC,
	ZRef<Callable> iCallable)
	{
	CreateStruct theCS = { iWNDPROC, iCallable };

	return ::CreateWindowExW
		(dwExStyle,
		spClassRegistration.GetClassName(),
		lpWindowName,
		dwStyle,
		X,
		Y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		ZUtil_Win::sGetModuleHandle(),
		&theCS); // creation parameters
	}

HWND sCreate(HWND iParent, ZRef<Callable> iCallable)
	{
	return sCreate
		(0, // Extended attributes
		nullptr, // window caption
		0, // window style
		0, // initial x position
		0, // initial y position
		10, // initial x size
		10, // initial y size
		iParent, // Parent window
		nullptr, // menu
		DefWindowProcW,
		iCallable);
	}

bool sAttach(HWND iHWND, ZRef<Callable> iCallable)
	{
	if (iCallable)
		{
		DWORD windowProcessID;
		::GetWindowThreadProcessId(iHWND, &windowProcessID);

		if (windowProcessID == ::GetCurrentProcessId())
			{
			WNDPROC basePROC = (WNDPROC)(LPARAM)
				::SetWindowLongPtrW(iHWND, GWLP_WNDPROC, (LPARAM)spWindowProcW);

			spAttach(iHWND, basePROC, iCallable);

			return true;
			}
		}
	return false;
	}

bool sDoOneMessage()
	{
	MSG theMSG;
	if (not ::GetMessageW(&theMSG, nullptr, 0, 0))
		return false;

	::TranslateMessage(&theMSG);
	::DispatchMessageW(&theMSG);

	return true;	
	}

// =================================================================================================
#pragma mark -
#pragma mark * ZWinWND, Callable <--> Dialog

HWND sCreateDialog(LPCWSTR lpTemplate, LCID iLCID, HWND hWndParent, ZRef<Callable_Dialog> iCallable)
	{
	HMODULE theHMODULE = ZUtil_Win::sGetModuleHandle();

	if (HRSRC theHRSRC = ::FindResourceExW(theHMODULE, (LPCWSTR)RT_DIALOG, lpTemplate, iLCID))
		{
		HGLOBAL theHGLOBAL = ::LoadResource(theHMODULE, theHRSRC);
		return ::CreateDialogIndirectParamW
			(theHMODULE,
			(LPCDLGTEMPLATE)::LockResource(theHGLOBAL),
			hWndParent,
			spDialogProcW,
			(LPARAM)iCallable.Get());
		}

	return ::CreateDialogParamW
		(theHMODULE,
		lpTemplate,
		hWndParent,
		spDialogProcW,
		(LPARAM)iCallable.Get());
	}

HWND sCreateDialog(LPCWSTR lpTemplate, HWND hWndParent, ZRef<Callable_Dialog> iCallable)
	{ return sCreateDialog(lpTemplate, ::GetThreadLocale(), hWndParent, iCallable); }

bool sDoOneMessageForDialog(HWND iHWND)
	{
	MSG theMSG;
	if (not ::GetMessageW(&theMSG, nullptr, 0, 0))
		return false;

	if (not ::IsDialogMessageW(iHWND, &theMSG))
		{
		::TranslateMessage(&theMSG);
		::DispatchMessageW(&theMSG);
		}

	return true;	
	}

} // namespace ZWinWND
} // namespace ZooLib

#endif // ZCONFIG_SPI_Enabled(Win)
