/* -------------------------------------------------------------------------------------------------
Copyright (c) 2007 Andrew Green
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

#include "ZStream_bzip2.h"

#if ZCONFIG_API_Enabled(Stream_bzip2)

#include "ZBitStream.h"
#include "ZCompat_algorithm.h" // For min/max
#include "ZDebug.h"

#include <stdexcept> // For runtime_error

using std::min;
using std::max;
using std::runtime_error;

#define kDebug_bzip2 2

// =================================================================================================
#pragma mark -
#pragma mark * ZStreamR_bzip2Decode

/**
\class ZStreamR_bzip2Decode
\ingroup stream
*/

/**
\param iStreamSource The stream from which compressed data will be read.
*/
ZStreamR_bzip2Decode::ZStreamR_bzip2Decode(const ZStreamR& iStreamSource)
:	fStreamSource(iStreamSource)
	{
	this->Internal_Init(1024);
	}

/**
\param iBufferSize The buffer size determines the maximum size of chunks we try to read from
the source stream. The default value is 1024 bytes which should be okay for most purposes. If
your source stream has poor latency then use a larger value or interpose a ZStreamR_Buffered.
\param iStreamSource The stream from which compressed data will be read.
*/
ZStreamR_bzip2Decode::ZStreamR_bzip2Decode(size_t iBufferSize, const ZStreamR& iStreamSource)
:	fStreamSource(iStreamSource)
	{
	this->Internal_Init(iBufferSize);
	}

ZStreamR_bzip2Decode::~ZStreamR_bzip2Decode()
	{
	::BZ2_bzDecompressEnd(&fState);
	delete[] fBuffer;
	}

void ZStreamR_bzip2Decode::Imp_Read(void* iDest, size_t iCount, size_t* oCountRead)
	{
	ZAssertStop(kDebug_bzip2, fState.avail_in == 0);
	fState.avail_out = iCount;
	fState.next_out = static_cast<char*>(iDest);
	for (;;)
		{
		int result = ::BZ2_bzDecompress(&fState);
		if (fState.avail_out == 0)
			{
			break;
			}
		else if (fState.avail_in == 0)
			{
			// Top up our input buffer
			size_t countReadable = fStreamSource.CountReadable();
			if (countReadable == 0)
				countReadable = 1;

			size_t countToRead = min(countReadable, fBufferSize);

			size_t countRead;
			fStreamSource.Read(fBuffer, countToRead, &countRead);

			if (countRead == 0)
				break;

			fState.avail_in = countRead;
			fState.next_in = fBuffer;
			}
		}
	if (oCountRead)
		*oCountRead = iCount - fState.avail_out;
	}

size_t ZStreamR_bzip2Decode::Imp_CountReadable()
	{
	return fState.avail_out;
	}

void ZStreamR_bzip2Decode::Internal_Init(size_t iBufferSize)
	{
	fBufferSize = max(size_t(1024), iBufferSize);
	fBuffer = new char[fBufferSize];

	fState.bzalloc = nil;
	fState.bzfree = nil;
	fState.opaque = nil;

	fState.next_in = fBuffer;
	fState.avail_in = 0;

	fState.next_out = nil;
	fState.avail_out = 0;

	if (BZ_OK != ::BZ2_bzDecompressInit(&fState, 0, 0))
		throw runtime_error("ZStreamR_bzip2Decode problem");
	}

// =================================================================================================
#pragma mark -
#pragma mark * ZStreamW_bzip2Encode

/**
\class ZStreamW_bzip2Encode
\ingroup stream
*/

/**
\param iBlockSize100K
\param iStreamSink The stream to which compressed data should be written.
*/
ZStreamW_bzip2Encode::ZStreamW_bzip2Encode(int iBlockSize100K, const ZStreamW& iStreamSink)
:	fStreamSink(iStreamSink)
	{
	this->Internal_Init(iBlockSize100K, 1024);
	}

/**
\param iBlockSize100K
\param iBufferSize. The buffer size determines how much data we accumulate before passing it on
to the destination stream. The default is 1024, and although almost any value will do we never
use less than 1024. If your destination stream has poor latency then use a (much) larger
value, or interpose a ZStreamW_Buffered or ZStreamW_DynamicBuffered.
\param iStreamSink The stream to which compressed data should be written.
*/
ZStreamW_bzip2Encode::ZStreamW_bzip2Encode(
	int iBlockSize100K, size_t iBufferSize, const ZStreamW& iStreamSink)
:	fStreamSink(iStreamSink)
	{
	this->Internal_Init(iBlockSize100K, iBufferSize);
	}

ZStreamW_bzip2Encode::~ZStreamW_bzip2Encode()
	{
	if (fBufferSize)
		{
		try
			{
			this->Internal_Flush();
			}
		catch (...)
			{}
		}

	::BZ2_bzCompressEnd(&fState);

	delete[] fBuffer;
	}

void ZStreamW_bzip2Encode::Imp_Write(const void* iSource, size_t iCount, size_t* oCountWritten)
	{
	if (!fBufferSize)
		{
		if (oCountWritten)
			*oCountWritten = 0;
		return;
		}

	ZAssertStop(kDebug_bzip2, fState.avail_in == 0);

	fState.avail_in = iCount;
	fState.next_in = const_cast<char*>(static_cast<const char*>(iSource));
	for (;;)
		{
		int result = ::BZ2_bzCompress(&fState, BZ_RUN);
		if (size_t countToWrite = fBufferSize - fState.avail_out)
			{
			size_t countWritten;
			fStreamSink.Write(fBuffer, countToWrite, &countWritten);
			if (countWritten < countToWrite)
				{
				// fStreamSink has closed. Mark ourselves similarly, by zeroing fBufferSize.
				fBufferSize = 0;
				break;
				}
			fState.next_out = fBuffer;
			fState.avail_out = fBufferSize;
			}
		else if (fState.avail_in == 0)
			{
			break;
			}
		}
	if (oCountWritten)
		*oCountWritten = iCount - fState.avail_in;
	}

/**
Flush closes off any pending zlib block, passes the data to our destination stream and
then flushes the destination stream. Calling Flush too often will degrade compression
performance. However, if the destination stream is being read from 'live' then you must
call Flush to ensure that pending data is passed on, rather than being buffered by zlib
in the hopes that it can be compressed in conjunction with subsequent data.
*/
void ZStreamW_bzip2Encode::Imp_Flush()
	{
	if (!fBufferSize)
		return;
	this->Internal_Flush();
	fStreamSink.Flush();
	}

void ZStreamW_bzip2Encode::Internal_Init(int iBlockSize100K, size_t iBufferSize)
	{
	fBufferSize = max(size_t(1024), iBufferSize);
	fBuffer = new char[fBufferSize];

	fState.bzalloc = nil;
	fState.bzfree = nil;
	fState.opaque = nil;

	fState.next_in = nil;
	fState.avail_in = 0;

	fState.next_out = fBuffer;
	fState.avail_out = fBufferSize;

	if (BZ_OK != ::BZ2_bzCompressInit(&fState, iBlockSize100K, 0, 0))
		throw runtime_error("ZStreamW_bzip2Encode problem");
	}

void ZStreamW_bzip2Encode::Internal_Flush()
	{
	ZAssertStop(kDebug_bzip2, fState.avail_in == 0);

	for (;;)
		{
		int result = ::BZ2_bzCompress(&fState, BZ_FLUSH);
		if (size_t countToWrite = fBufferSize - fState.avail_out)
			{
			size_t countWritten;
			fStreamSink.Write(fBuffer, countToWrite, &countWritten);
			if (countWritten < countToWrite)
				{
				fBufferSize = 0;
				ZStreamW::sThrowEndOfStream();
				}
			fState.next_out = fBuffer;
			fState.avail_out = fBufferSize;
			}
		else
			{
			break;
			}
		}
	}

// =================================================================================================
#pragma mark -
#pragma mark * ZStreamerW_bzip2Encode

ZStreamerW_bzip2Encode::ZStreamerW_bzip2Encode(int iBlockSize100K, ZRef<ZStreamerW> iStreamer)
:	fStreamer(iStreamer),
	fStream(iBlockSize100K, iStreamer->GetStreamW())
	{}

ZStreamerW_bzip2Encode::ZStreamerW_bzip2Encode(
	int iBlockSize100K, size_t iBufferSize, ZRef<ZStreamerW> iStreamer)
:	fStreamer(iStreamer),
	fStream(iBlockSize100K, iBufferSize, iStreamer->GetStreamW())
	{}

ZStreamerW_bzip2Encode::~ZStreamerW_bzip2Encode()
	{}

const ZStreamW& ZStreamerW_bzip2Encode::GetStreamW()
	{ return fStream; }

// =================================================================================================
#pragma mark -
#pragma mark * ZStream_bzip2

static const uint64 BLOCK_HEADER = 0x314159265359ULL;
static const uint64 BLOCK_ENDMARK = 0x177245385090ULL;
static const uint64 BLOCK_MASK = 0xFFFFFFFFFFFFULL;

void ZStream_bzip2::sAnalyze(const ZStreamR& iStreamR, vector<pair<uint64, uint32> >& oOffsets)
	{
	oOffsets.clear();
	oOffsets.reserve(1024);

	ZBitReaderLE r;

	uint64 bitStartCurrent = 0;
	uint64 bitsRead = 0;
	uint64 buffer = 0;
	bool inBlock = false;
	for (;;)
		{
		uint32 bit;
		if (!r.ReadBits(iStreamR, 1, bit))
			break;

		buffer = ((buffer << 1) | bit) & BLOCK_MASK;
		if (buffer == BLOCK_ENDMARK)
			{
			if (inBlock)
				oOffsets.push_back(pair<uint64, uint32>(bitStartCurrent, bitsRead - bitStartCurrent));
			inBlock = false;
			}
		else if (buffer == BLOCK_HEADER)
			{
			if (inBlock)
				oOffsets.push_back(pair<uint64, uint32>(bitStartCurrent, bitsRead - bitStartCurrent));
			inBlock = true;
			bitStartCurrent = bitsRead;
			}
		++bitsRead;
		}
	}

#endif // ZCONFIG_API_Enabled(Stream_bzip2)
