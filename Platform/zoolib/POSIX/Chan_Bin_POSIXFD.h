// Copyright (c) 2018 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_POSIX_Chan_Bin_POSIXFD_h__
#define __ZooLib_POSIX_Chan_Bin_POSIXFD_h__ 1
#include "zconfig.h"
#include "zoolib/ZCONFIG_API.h"
#include "zoolib/ZCONFIG_SPI.h"

#include "zoolib/ChanR_Bin.h"
#include "zoolib/ChanW_Bin.h"
#include "zoolib/Counted.h"

#if ZCONFIG_SPI_Enabled(POSIX)

namespace ZooLib {

// =================================================================================================
#pragma mark - FDHolder

class FDHolder
:	public Counted
	{
public:
	FDHolder();
	virtual ~FDHolder();

// Our protocol
	virtual int GetFD() = 0;
	};

// =================================================================================================
#pragma mark - FDHolder_Std

class FDHolder_Std
:	public FDHolder
	{
public:
	FDHolder_Std(int iFD);
	virtual ~FDHolder_Std();

// Our protocol
	virtual int GetFD();

protected:
	int fFD;
	};

// =================================================================================================
#pragma mark - FDHolder_CloseOnDestroy

class FDHolder_CloseOnDestroy
:	public FDHolder
	{
public:
	FDHolder_CloseOnDestroy(int iFD);
	virtual ~FDHolder_CloseOnDestroy();

// Our protocol
	virtual int GetFD();

protected:
	int fFD;
	};

// =================================================================================================
#pragma mark - ChanR_Bin_POSIXFD

class ChanR_Bin_POSIXFD
:	public ChanR<byte>
	{
	ChanR_Bin_POSIXFD(const ZP<FDHolder>& iFDHolder);
	~ChanR_Bin_POSIXFD();

// From ChanAspect_Read<byte>
	virtual size_t Read(byte* oDest, size_t iCount);
	virtual size_t Readable();

protected:
	const ZP<FDHolder> fFDHolder;
	};

// =================================================================================================
#pragma mark - ChanW_Bin_POSIXFD

class ChanW_Bin_POSIXFD
:	public ChanW_Bin
	{
public:
	ChanW_Bin_POSIXFD(const ZP<FDHolder>& iFDHolder);
	~ChanW_Bin_POSIXFD();

// From ChanAspect_Write<byte>
	virtual size_t Write(const byte* iSource, size_t iCount);

protected:
	const ZP<FDHolder> fFDHolder;
	};

// =================================================================================================
#pragma mark - ChanRPos_Bin_POSIXFD

class ChanRPos_Bin_POSIXFD
:	public ChanRPos<byte>
	{
public:
	ChanRPos_Bin_POSIXFD(const ZP<FDHolder>& iFDHolder);
	~ChanRPos_Bin_POSIXFD();

// From Aspect Pos
	virtual uint64 Pos();
	virtual void PosSet(uint64 iPos);

// From ChanAspect_Read<byte>
	virtual size_t Read(byte* oDest, size_t iCount);
	virtual uint64 Skip(uint64 iCount);
	virtual size_t Readable();

// From ChanAspect_Size
	virtual uint64 Size();

// From ChanAspect_Unread<byte>
	virtual size_t Unread(const byte* iSource, size_t iCount);

protected:
	const ZP<FDHolder> fFDHolder;
	};

// =================================================================================================
#pragma mark - ChanWPos_Bin_POSIXFD

class ChanWPos_Bin_POSIXFD
:	public ChanWPos<byte>
	{
public:
	ChanWPos_Bin_POSIXFD(const ZP<FDHolder>& iFDHolder);
	~ChanWPos_Bin_POSIXFD();

// From Aspect Pos
	virtual uint64 Pos();
	virtual void PosSet(uint64 iPos);

// From ChanAspect_Size
	virtual uint64 Size();

// From ChanAspect_SizeSet
	virtual void SizeSet(uint64 iSize);

// From ChanAspect_Write<byte>
	virtual size_t Write(const byte* iSource, size_t iCount);

protected:
	const ZP<FDHolder> fFDHolder;
	};

// =================================================================================================
#pragma mark - ChanRWPos_Bin_POSIXFD

class ChanRWPos_Bin_POSIXFD
:	public ChanRWPos<byte>
	{
public:
	ChanRWPos_Bin_POSIXFD(const ZP<FDHolder>& iFDHolder);
	~ChanRWPos_Bin_POSIXFD();

// From Aspect Pos
	virtual uint64 Pos();
	virtual void PosSet(uint64 iPos);

// From ChanAspect_Read<byte>
	virtual size_t Read(byte* oDest, size_t iCount);
	virtual size_t Readable();

// From ChanAspect_Size
	virtual uint64 Size();

// From ChanAspect_SizeSet
	virtual void SizeSet(uint64 iSize);

// From ChanAspect_Write<byte>
	virtual size_t Write(const byte* iSource, size_t iCount);

// From ChanAspect_Unread<byte>
	virtual size_t Unread(const byte* iSource, size_t iCount);

protected:
	const ZP<FDHolder> fFDHolder;
	};

// =================================================================================================
#pragma mark - ChanRAbort_Bin_POSIXFD

class ChanRAbort_Bin_POSIXFD
:	public ChanRAbort<byte>
	{
public:
	ChanRAbort_Bin_POSIXFD(const ZP<FDHolder>& iFDHolder);
	~ChanRAbort_Bin_POSIXFD();

// From ChanAspect_Abort
	virtual void Abort();

// From ChanAspect_Read<byte>
	virtual size_t Read(byte* oDest, size_t iCount);
	virtual size_t Readable();

// From ChanAspect_WaitReadable
	virtual bool WaitReadable(double iTimeout);

protected:
	const ZP<FDHolder> fFDHolder;
	};

// =================================================================================================
#pragma mark - ChanWAbort_Bin_POSIXFD

class ChanWAbort_Bin_POSIXFD
:	public ChanWAbort<byte>
	{
public:
	ChanWAbort_Bin_POSIXFD(const ZP<FDHolder>& iFDHolder);
	~ChanWAbort_Bin_POSIXFD();

// From ChanAspect_Abort
	virtual void Abort();

// From ChanAspect_Write<byte>
	virtual size_t Write(const byte* iSource, size_t iCount);

protected:
	const ZP<FDHolder> fFDHolder;
	};

// =================================================================================================
#pragma mark - ChanRWAbort_Bin_POSIXFD

class ChanRWAbort_Bin_POSIXFD
:	public ChanRWAbort<byte>
	{
public:
	ChanRWAbort_Bin_POSIXFD(const ZP<FDHolder>& iFDHolder);
	~ChanRWAbort_Bin_POSIXFD();

// From ChanAspect_Abort
	virtual void Abort();

// From ChanAspect_Read<byte>
	virtual size_t Read(byte* oDest, size_t iCount);
	virtual size_t Readable();

// From ChanAspect_WaitReadable
	virtual bool WaitReadable(double iTimeout);

// From ChanAspect_Write<byte>
	virtual size_t Write(const byte* iSource, size_t iCount);

protected:
	const ZP<FDHolder> fFDHolder;
	};

} // namespace ZooLib

#endif // ZCONFIG_SPI_Enabled(POSIX)

#endif // __ZooLib_POSIX_Chan_Bin_POSIXFD_h__
