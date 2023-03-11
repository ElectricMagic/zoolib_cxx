// Copyright (c) 2003 Andrew Green and Learning in Motion, Inc.
// MIT License. http://www.zoolib.org

#include "zoolib/POSIX/FILE_Channer.h"

#include "zoolib/ChanR_Bin.h"
#include "zoolib/ChanW_Bin.h"

#include "zoolib/ZCONFIG_SPI.h"

#if ZCONFIG_SPI_Enabled(BSD) || (defined(__ANDROID__) && defined(__USE_BSD))
	#define ZMACRO_Use_funopen 1
#endif

#ifndef ZMACRO_Use_funopen
	#define ZMACRO_Use_funopen 0
#endif

namespace ZooLib {

// =================================================================================================
#pragma mark - FILE backed by a Channer

#if 0

#elif ZMACRO_Use_funopen

// FIXME. If __ANDROID_API__>24 we have funopen64, which has fpos64_t.

static int spReadR(void* iCookie, char* oDest, int iCount)
	{ return (int)sReadMem(*static_cast<ChannerR_Bin*>(iCookie), oDest, iCount); }

static int spReadRPos(void* iCookie, char* oDest, int iCount)
	{ return (int)sReadMem(*static_cast<ChannerRPos_Bin*>(iCookie), oDest, iCount); }

static int spWrite(void* iCookie, const char* iSource, int iCount)
	{ return (int)sWriteMem(*static_cast<ChannerW_Bin*>(iCookie), iSource, iCount); }

static fpos_t spSeek(void* iCookie, fpos_t iPos, int iWhence)
	{
	ChannerRPos_Bin* theChannerRPos = static_cast<ChannerRPos_Bin*>(iCookie);
	switch (iWhence)
		{
		case SEEK_SET:
			{
			sPosSet(*theChannerRPos, iPos);
			return iPos;
			}
		case SEEK_CUR:
			{
			uint64 newPos = sPos(*theChannerRPos) + iPos;
			sPosSet(*theChannerRPos, newPos);
			return fpos_t(newPos);
			}
		case SEEK_END:
			{
			uint64 newPos = sSize(*theChannerRPos) + iPos;
			sPosSet(*theChannerRPos, newPos);
			return fpos_t(newPos);
			}
		}
	ZUnimplemented();
	return -1;
	}

static int spCloseR(void* iCookie)
	{
	static_cast<ChannerR_Bin*>(iCookie)->Release();
	return 0;
	}

static int spCloseRPos(void* iCookie)
	{
	static_cast<ChannerRPos_Bin*>(iCookie)->Release();
	return 0;
	}

static int spCloseW(void* iCookie)
	{
	static_cast<ChannerW_Bin*>(iCookie)->Release();
	return 0;
	}

FILE* sFILE_R(ZP<ChannerR_Bin> iChannerR)
	{
	if (iChannerR)
		{
		iChannerR->Retain();
		return ::funopen(iChannerR.Get(), spReadR, nullptr, nullptr, spCloseR);
		}
	return nullptr;
	}

FILE* sFILE_RPos(ZP<ChannerRPos_Bin> iChannerRPos)
	{
	if (iChannerRPos)
		{
		iChannerRPos->Retain();
		return ::funopen(iChannerRPos.Get(), spReadRPos, nullptr, spSeek, spCloseRPos);
		}
	return nullptr;
	}

FILE* sFILE_W(ZP<ChannerW_Bin> iChannerW)
	{
	if (iChannerW)
		{
		iChannerW->Retain();
		return ::funopen(iChannerW.Get(), nullptr, spWrite, nullptr, spCloseW);
		}
	return nullptr;
	}

#elif defined(__USE_GNU)

static ssize_t spReadR(void* iCookie, char* oDest, size_t iCount)
	{ return sReadMem(*static_cast<ChannerR_Bin*>(iCookie), oDest, iCount); }

static ssize_t spReadRPos(void* iCookie, char* oDest, size_t iCount)
	{ return sReadMem(*static_cast<ChannerRPos_Bin*>(iCookie), oDest, iCount); }

static ssize_t spWrite(void* iCookie, const char* iSource, size_t iCount)
	{ return sWriteMem(*static_cast<ChannerW_Bin*>(iCookie), iSource, iCount); }

static int spSeek(void* iCookie, off64_t *ioPos, int iWhence)
	{
	ChannerRPos_Bin* theChannerRPos = static_cast<ChannerRPos_Bin*>(iCookie);
	switch (iWhence)
		{
		case SEEK_SET:
			{
			sPosSet(*theChannerRPos, *ioPos);
			return 0;
			}
		case SEEK_CUR:
			{
			*ioPos += sPos(*theChannerRPos);
			sPosSet(*theChannerRPos, *ioPos);
			return 0;
			}
		case SEEK_END:
			{
			*ioPos += sSize(*theChannerRPos);
			sPosSet(*theChannerRPos, *ioPos);
			return 0;
			}
		}
	ZUnimplemented();
	return -1;
	}

static int spCloseR(void* iCookie)
	{
	static_cast<ChannerR_Bin*>(iCookie)->Release();
	return 0;
	}

static int spCloseRPos(void* iCookie)
	{
	static_cast<ChannerRPos_Bin*>(iCookie)->Release();
	return 0;
	}

static int spCloseW(void* iCookie)
	{
	static_cast<ChannerW_Bin*>(iCookie)->Release();
	return 0;
	}

FILE* sFILE_R(ZP<ChannerR_Bin> iChannerR)
	{
	if (iChannerR)
		{
		iChannerR->Retain();

		_IO_cookie_io_functions_t theFunctions;
		theFunctions.read = spReadR;
		theFunctions.write = nullptr;
		theFunctions.seek = nullptr;
		theFunctions.close = spCloseR;
		return ::fopencookie(iChannerR.Get(), "", theFunctions);
		}
	return nullptr;
	}

FILE* sFILE_RPos(ZP<ChannerRPos_Bin> iChannerRPos)
	{
	if (iChannerRPos)
		{
		iChannerRPos->Retain();

		_IO_cookie_io_functions_t theFunctions;
		theFunctions.read = spReadRPos;
		theFunctions.write = nullptr;
		theFunctions.seek = spSeek;
		theFunctions.close = spCloseRPos;
		return ::fopencookie(iChannerRPos.Get(), "", theFunctions);
		}
	return nullptr;
	}

FILE* sFILE_W(ZP<ChannerW_Bin> iChannerW)
	{
	if (iChannerW)
		{
		iChannerW->Retain();

		_IO_cookie_io_functions_t theFunctions;
		theFunctions.read = nullptr;
		theFunctions.write = spWrite;
		theFunctions.seek = nullptr;
		theFunctions.close = spCloseW;
		return ::fopencookie(iChannerW.Get(), "", theFunctions);
		}
	return nullptr;
	}

#else

FILE* sFILE_R(ZP<ChannerR_Bin> iChannerR)
	{ return nullptr; }

FILE* sFILE_RPos(ZP<ChannerRPos_Bin> iChannerRPos)
	{ return nullptr; }

FILE* sFILE_W(ZP<ChannerW_Bin> iChannerW)
	{ return nullptr; }

#endif

} // namespace ZooLib
