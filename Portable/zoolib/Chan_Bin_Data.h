// Copyright (c) 2014 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_Chan_Bin_Data_h__
#define __ZooLib_Chan_Bin_Data_h__ 1
#include "zconfig.h"

#include "zoolib/ChanR_Bin.h"
#include "zoolib/ChanW_Bin.h"
#include "zoolib/Util_Chan.h" // For sCopyAll

namespace ZooLib {

// =================================================================================================
#pragma mark - ChanRPos_Bin_Data

template <class Data_p>
class ChanRPos_Bin_Data
:	public ChanRPos_Bin
	{
public:
	typedef Data_p Data;

	ChanRPos_Bin_Data(const Data& iData)
	:	fData(iData)
	,	fPosition(0)
		{}

// From ChanPos
	virtual uint64 Pos()
		{ return fPosition; }

	virtual void PosSet(uint64 iPos)
		{ fPosition = sClamped(iPos); }

// From ChanR
	virtual size_t Read(byte* oDest, size_t iCount)
		{
		const size_t theSize = fData.GetSize();
		const size_t countToCopy = std::min<size_t>(iCount,
			theSize > fPosition ? theSize - fPosition : 0);
		fData.CopyTo(fPosition, oDest, countToCopy);
		fPosition += countToCopy;
		return countToCopy;
		}

	virtual size_t Readable()
		{
		const size_t theSize = fData.GetSize();
		return theSize >= fPosition ? theSize - fPosition : 0;
		}

// From ChanSize
	virtual uint64 Size()
		{ return fData.GetSize(); }

// From ChanU
	virtual size_t Unread(const byte* iSource, size_t iCount)
		{
		const size_t countToCopy = std::min(iCount, this->fPosition);
		this->fPosition -= countToCopy;

		// See Chan_XX_Memory for a note regarding bogus unreads.

		return countToCopy;
		}

private:
	Data fData;
	size_t fPosition;
	};

// =================================================================================================
#pragma mark - ChanRWPos_Bin_Data

template <class Data_p>
class ChanRWPos_Bin_Data
:	public ChanRWPos<byte>
	{
public:
	typedef Data_p Data;

	ChanRWPos_Bin_Data()
	:	fDataPtr(&fDataMaybe)
	,	fPosition(0)
		{}

	ChanRWPos_Bin_Data(const Data& iData)
	:	fDataMaybe(iData)
	,	fDataPtr(&fDataMaybe)
	,	fPosition(0)
		{}

	ChanRWPos_Bin_Data(Data* ioData)
	:	fDataPtr(ioData)
	,	fPosition(0)
		{}

// From ChanPos
	virtual uint64 Pos()
		{ return fPosition; }

	virtual void PosSet(uint64 iPos)
		{ fPosition = sClamped(iPos); }

// From ChanR
	virtual size_t Read(byte* oDest, size_t iCount)
		{
		const size_t theSize = fDataPtr->GetSize();
		const size_t countToCopy = std::min<size_t>(iCount,
			theSize > fPosition ? theSize - fPosition : 0);
		fDataPtr->CopyTo(fPosition, oDest, countToCopy);
		fPosition += countToCopy;
		return countToCopy;
		}

	virtual size_t Readable()
		{
		const size_t theSize = fDataPtr->GetSize();
		return theSize >= fPosition ? theSize - fPosition : 0;
		}

// From ChanSize
	virtual uint64 Size()
		{ return fDataPtr->GetSize(); }

// From ChanSizeSet
	virtual void SizeSet(uint64 iSize)
		{
		size_t actualSize = sClamped(iSize);
		if (fPosition > actualSize)
			fPosition = actualSize;
		fDataPtr->SetSize(actualSize);
		}

// From ChanU
	virtual size_t Unread(const byte* iSource, size_t iCount)
		{
		const size_t countToCopy = std::min<size_t>(iCount, this->fPosition);

		this->fDataPtr->CopyFrom(this->fPosition - countToCopy, iSource, countToCopy);

		this->fPosition -= countToCopy;

		return countToCopy;
		}

// From ChanW
	virtual size_t Write(const byte* iSource, size_t iCount)
		{
		const size_t newPosition = fPosition + iCount;
		if (fDataPtr->GetSize() < newPosition)
			fDataPtr->SetSize(newPosition);

		fDataPtr->CopyFrom(fPosition, iSource, iCount);

		fPosition = newPosition;

		return iCount;
		}

// Our protocol
	Data GetData() const
		{ return *fDataPtr; }

	Data& MutData()
		{ return *fDataPtr; }

private:
	Data fDataMaybe;
	Data* fDataPtr;
	size_t fPosition;
	};

// =================================================================================================
#pragma mark - ChanW_Bin_Data

template <class Data_p>
class ChanW_Bin_Data
:	public ChanW<byte>
	{
public:
	typedef Data_p Data;

	ChanW_Bin_Data(Data* ioData)
	:	fDataPtr(ioData)
		{}

	~ChanW_Bin_Data()
		{}

// From ChanW
	virtual size_t Write(const byte* iSource, size_t iCount)
		{
		const size_t thePos = fDataPtr->GetSize();
		fDataPtr->SetSize(thePos + iCount);

		fDataPtr->CopyFrom(thePos, iSource, iCount);

		return iCount;
		}

private:
	Data* fDataPtr;
	};

// =================================================================================================
#pragma mark - Data stream reading functions

template <class Data_p>
Data_p sReadAll_T(const ChanR_Bin& iChanR)
	{
	Data_p theData;
	sECopyAll(iChanR, ChanW_Bin_Data<Data_p>(&theData));
	return theData;
	}

template <class Data_p>
Data_p sRead_T(const ChanR_Bin& iChanR, size_t iSize)
	{
	Data_p theData(iSize);
	sEReadMem(iChanR, theData.GetPtrMutable(), iSize);
	return theData;
	}

} // namespace ZooLib

#endif // __ZooLib_Chan_Bin_Data_h__
