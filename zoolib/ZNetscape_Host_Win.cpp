/* -------------------------------------------------------------------------------------------------
Copyright (c) 2009 Andrew Green
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

#include "zoolib/ZNetscape_Host_Win.h"

#if defined(XP_WIN)

#include "zoolib/ZLog.h"
#include "zoolib/ZMemory.h"

#include <cstdio>

NAMESPACE_ZOOLIB_BEGIN

namespace ZNetscape {

// =================================================================================================
#pragma mark -
#pragma mark * Host_Win

Host_Win::Host_Win(ZRef<GuestFactory> iGuestFactory, HWND iHWND)
:	Host_Std(iGuestFactory),
	fWND(iHWND),
	fIsWindowed(true),
	fIsTransparent(false),
	fTimerID(0),
	fInnerWND(nullptr)
	{
	ZBlockZero(&fNPWindow, sizeof(fNPWindow));
	this->Attach(fWND);
	fTimerID = ::SetTimer(fWND, 1, 50, nullptr);
	}

Host_Win::~Host_Win()
	{}

NPError Host_Win::Host_GetValue(NPP npp, NPNVariable variable, void* ret_value)
	{
	switch (variable)
		{
		case NPNVnetscapeWindow:
			{
			if (fIsWindowed)
				*static_cast<HWND*>(ret_value) = fInnerWND;
			else
				*static_cast<HWND*>(ret_value) = fWND;
			return NPERR_NO_ERROR;
			}
		case NPNVSupportsWindowless:
			{
			*static_cast<NPBool*>(ret_value) = TRUE;
            return NPERR_NO_ERROR;
			}
		}

	return Host_Std::Host_GetValue(npp, variable, ret_value);
	}

NPError Host_Win::Host_SetValue(NPP npp, NPPVariable variable, void* value)
	{
	switch (variable)
		{
		case NPPVpluginWindowBool:
			{
			if (ZLOG(s, eDebug, "Host_Win"))
				s << "Host_SetValue, NPPVpluginWindowBool: " << (value ? "true" : "false");
			fIsWindowed = value;
			return NPERR_NO_ERROR;
			}
		case NPPVpluginTransparentBool:
			{
			if (ZLOG(s, eDebug, "Host_Win"))
				s << "Host_SetValue, NPPVpluginTransparentBool: " << (value ? "true" : "false");
			fIsTransparent = value;
			return NPERR_NO_ERROR;
			}
		}

	return Host_Std::Host_SetValue(npp, variable, value);
	}

void Host_Win::Host_InvalidateRect(NPP npp, NPRect* rect)
	{
	if (!fIsWindowed)
		{
		RECT theRECT = { rect->left, rect->top, rect->right, rect->bottom };
		::InvalidateRect(fWND, &theRECT, false);
		}
	}

void Host_Win::PostCreateAndLoad()
	{
	RECT theCR;
	::GetClientRect(fWND, &theCR);
	const int theWidth = theCR.right - theCR.left;
	const int theHeight = theCR.bottom - theCR.top;

	if (fIsWindowed)
		{
		fInnerWND = ZWNDA::sCreateDefault(fWND, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, nullptr);
		::SetWindowPos(fInnerWND, nullptr,
			0, 0,
			theWidth, theHeight,
			SWP_NOZORDER | SWP_SHOWWINDOW);

		fNPWindow.type = NPWindowTypeWindow;
		fNPWindow.window = fInnerWND;
		}
	else
		{
		fNPWindow.type = NPWindowTypeDrawable;
		fNPWindow.window = fInnerWND;
		}

	this->pStuffNPWindow(theWidth, theHeight);
	}

void Host_Win::PaintBackground(HDC iHDC, const PAINTSTRUCT& iPS)
	{}

LRESULT Host_Win::WindowProc(HWND iHWND, UINT iMessage, WPARAM iWPARAM, LPARAM iLPARAM)
	{
	// From WebCore's PluginViewWin.cpp
	switch (iMessage)
		{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			{
			::SetCapture(iHWND);
			break;
			}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			{
			::ReleaseCapture();
			break;
			}
		}

//--

	switch (iMessage)
		{
		case WM_TIMER:
			{
			this->DeliverData();
			if (!fIsWindowed)
				{
				NPEvent theNPEvent;
				theNPEvent.event = iMessage;
				theNPEvent.wParam = iWPARAM;
				theNPEvent.lParam = iLPARAM;
				this->Guest_HandleEvent(&theNPEvent);
				}
			return 0;
			}
		case WM_PAINT:
			{
			this->pPaint(iHWND, iWPARAM, iLPARAM);
			return 0;
			}
		case WM_WINDOWPOSCHANGED:
			{
			WINDOWPOS* theWPOS = reinterpret_cast<WINDOWPOS*>(iLPARAM);
			if (!(theWPOS->flags & SWP_NOSIZE))
				{
				this->pStuffNPWindow(
					theWPOS->cx,
					theWPOS->cy);

				if (fInnerWND)
					{
					::SetWindowPos(fInnerWND, nullptr,
						0, 0, theWPOS->cx, theWPOS->cy,
						SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
					}
				else
					{
					RECT theCR;
					::GetClientRect(iHWND, &theCR);
					::InvalidateRect(fHWND, &theCR, false);
					}
				}			
			break;
			}
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
//		case WM_MOUSEHOVER:
		case WM_MOUSEACTIVATE:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_DEADCHAR:
		case WM_SETCURSOR:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			{
			if (!fIsWindowed)
				{
				NPEvent theNPEvent;
				theNPEvent.event = iMessage;
				theNPEvent.wParam = iWPARAM;
				theNPEvent.lParam = iLPARAM;
				this->Guest_HandleEvent(&theNPEvent);
				return 0;				
				}
			break;
			}
		}

	return ZWNDSubClassW::WindowProc(iHWND, iMessage, iWPARAM, iLPARAM);
	}

void Host_Win::pPaint(HWND iHWND, WPARAM iWPARAM, LPARAM iLPARAM)
	{
	if (ZLOG(s, eDebug, "Host_Win"))
		s << "+DoPAINT";

	PAINTSTRUCT thePS;
	HDC theHDC = ::BeginPaint(iHWND, &thePS);
	RECT theCR;
	::GetClientRect(iHWND, &theCR);

	this->PaintBackground(theHDC, thePS);

	if (!fIsWindowed)
		{
		fNPWindow.window = theHDC;
		this->pStuffNPWindow(theCR.right - theCR.left, theCR.bottom - theCR.top);

		NPEvent theNPEvent;
		theNPEvent.event = WM_PAINT;
		theNPEvent.wParam = iWPARAM;
		theNPEvent.lParam = 0;

		this->Guest_HandleEvent(&theNPEvent);
		}

	::EndPaint(iHWND, &thePS);

	if (ZLOG(s, eDebug, "Host_Win"))
		s << "-DoPAINT";
	}

void Host_Win::pStuffNPWindow(int iWidth, int iHeight)
	{
	fNPWindow.x = 0;
	fNPWindow.y = 0;
	fNPWindow.width = iWidth;
	fNPWindow.height = iHeight;
	fNPWindow.clipRect.left = 0;
	fNPWindow.clipRect.top = 0;
	fNPWindow.clipRect.right = iWidth;
	fNPWindow.clipRect.bottom = iHeight;

	this->Guest_SetWindow(&fNPWindow);
	}

} // namespace ZNetscape

NAMESPACE_ZOOLIB_END

#endif // defined(XP_WIN)
