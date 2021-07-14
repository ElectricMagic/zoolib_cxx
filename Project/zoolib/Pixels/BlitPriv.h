// Copyright (c) 2005 Andrew Green and Learning in Motion, Inc.
// MIT License. http://www.zoolib.org

#ifndef __ZooLib_Pixels_BlitPriv_h__
#define __ZooLib_Pixels_BlitPriv_h__ 1
#include "zconfig.h"

#include "zoolib/Pixels/Blit.h" // For EOp and abbreviating typedefs
#include "zoolib/Pixels/PixelIters.h"
#include "zoolib/Pixels/Cartesian_Geom.h"

namespace ZooLib {
namespace Pixels {

// =================================================================================================
// MARK: - Helpers

// Returns a number between 0 and iRange - 1. The effect is as if enough multiples
// of iRange are added to or subtracted from iAmount to bring it into that range.
// Note that iRange should be positive.

inline int sPositiveModulus(int iAmount, int iRange)
	{
	return ((iAmount % iRange) + iRange) % iRange;
	}

// =================================================================================================
// MARK: - Porter-Duff composition functors

/*
Porter-Duff operators

f, b are rgb tuples
color is red, green or blue component, premultiplied with alpha
alpha is alpha component

f over b	f.color + (1 - f.alpha) * b.color
f in b		f.color * b.alpha
f out b		f.color * (1 - b.alpha)
f atop b	f.color * b.alpha + b.color * (1 - f.alpha)
f xor b		f.color.color * (1 - b.alpha) + b.color * (1 - f.alpha)
f plus b	f.color + b.color
*/

struct Compose_Over
	{
	void operator() (const RGBA& f, RGBA& b) const
		{
		Comp oneMinusAlpha = 1 - sAlpha(f);
		sRed(b) = sRed(b) * oneMinusAlpha + sRed(f);
		sGreen(b) = sGreen(b) * oneMinusAlpha + sGreen(f);
		sBlue(b) = sBlue(b) * oneMinusAlpha + sBlue(f);
		sAlpha(b) = sAlpha(b) * oneMinusAlpha + sAlpha(f);
		}
	};

struct Compose_Plus
	{
	void operator() (const RGBA& f, RGBA& b) const
		{
		// This will crap out with luminescent pixels.
		b += f;
		}
	};

// =================================================================================================
// MARK: - Tile variants

template <class S, class D>
void sTile_SD_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int sourceLeft = iSourceF.left;
	int sourceWidth = W(iSourceF);
	int sourceHStart = iSourceF.left + sPositiveModulus(iSourceOrigin.h, sourceWidth);
	int sourceCountHStart = iSourceF.right - sourceHStart;
	int sourceCountHReset = sourceWidth;

	int sourceTop = iSourceF.top;
	int sourceBottom = iSourceF.bottom;
	int sourceHeight = H(iSourceF);
	int sourceVStart = iSourceF.top + sPositiveModulus(iSourceOrigin.v, sourceHeight);

	int sourceV = sourceVStart;
	const void* sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sourceRowAddress,
		sourceHStart,
		iSourcePD);

	int currentDestV = iDestF.top;
	PixelIterW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int sourceCountH = sourceCountHStart;

		while (--destCountH)
			{
			RGBA thePixel = sourceIter.ReadInc();
			destIter.WriteInc(thePixel);

			if (not --sourceCountH)
				{
				sourceIter.Reset(sourceRowAddress, sourceLeft);
				sourceCountH = sourceCountHReset;
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);

 		++sourceV;
		if (sourceV >= sourceBottom)
			sourceV = sourceTop;

		sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
		sourceIter.Reset(sourceRowAddress, sourceHStart);
		}
	}

template <class S, class D, class O>
void sTile_SDO_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const O& iOp)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int sourceLeft = iSourceF.left;
	int sourceWidth = W(iSourceF);
	int sourceHStart = iSourceF.left + sPositiveModulus(iSourceOrigin.h, sourceWidth);
	int sourceCountHStart = iSourceF.right - sourceHStart;
	int sourceCountHReset = sourceWidth;

	int sourceTop = iSourceF.top;
	int sourceBottom = iSourceF.bottom;
	int sourceHeight = H(iSourceF);
	int sourceVStart = iSourceF.top + sPositiveModulus(iSourceOrigin.v, sourceHeight);

	int sourceV = sourceVStart;

	const void* sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sourceRowAddress,
		sourceHStart,
		iSourcePD);

	int currentDestV = iDestF.top;
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int sourceCountH = sourceCountHStart;

		while (--destCountH)
			{
			RGBA thePixel = sourceIter.ReadInc();
			RGBA destPixel = destIter.Read();
			iOp(thePixel, destPixel);
			destIter.WriteInc(destPixel);

			if (not --sourceCountH)
				{
				sourceIter.Reset(sourceRowAddress, sourceLeft);
				sourceCountH = sourceCountHReset;
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++sourceV;
		if (sourceV >= sourceBottom)
			sourceV = sourceTop;

		sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
		sourceIter.Reset(sourceRowAddress, sourceHStart);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

template <class S, class M, class D>
void sTile_SMD_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int sourceLeft = iSourceF.left;
	int sourceWidth = W(iSourceF);
	int sourceHStart = iSourceF.left + sPositiveModulus(iSourceOrigin.h, sourceWidth);
	int sourceCountHStart = iSourceF.right - sourceHStart;
	int sourceCountHReset = sourceWidth;

	int sourceTop = iSourceF.top;
	int sourceBottom = iSourceF.bottom;
	int sourceHeight = H(iSourceF);
	int sourceVStart = iSourceF.top + sPositiveModulus(iSourceOrigin.v, sourceHeight);

	int sourceV = sourceVStart;

	int matteLeft = iMatteF.left;
	int matteWidth = W(iMatteF);
	int matteHStart = iMatteF.left + sPositiveModulus(iMatteOrigin.h, matteWidth);
	int matteCountHStart = iMatteF.right - matteHStart;
	int matteCountHReset = matteWidth;

	int matteTop = iMatteF.top;
	int matteBottom = iMatteF.bottom;
	int matteHeight = H(iMatteF);
	int matteVStart = iMatteF.top + sPositiveModulus(iMatteOrigin.v, matteHeight);

	int matteV = matteVStart;

	const void* sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sourceRowAddress,
		sourceHStart,
		iSourcePD);

	const void* matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		matteRowAddress,
		matteHStart,
		iMattePD);

	int currentDestV = iDestF.top;
	PixelIterW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int sourceCountH = sourceCountHStart;
		int matteCountH = matteCountHStart;

		if (iSourcePremultiplied)
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();
				sAlpha(thePixel) = matteIter.ReadAlphaInc();
				destIter.WriteInc(thePixel);

				if (not --sourceCountH)
					{
					sourceIter.Reset(sourceRowAddress, sourceLeft);
					sourceCountH = sourceCountHReset;
					}

				if (not --matteCountH)
					{
					matteIter.Reset(matteRowAddress, matteLeft);
					matteCountH = matteCountHReset;
					}
				}
			}
		else
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();

				Comp alpha = matteIter.ReadAlphaInc();
				sRed(thePixel) *= alpha;
				sGreen(thePixel) *= alpha;
				sBlue(thePixel) *= alpha;
				sAlpha(thePixel) = alpha;

				destIter.WriteInc(thePixel);

				if (not --sourceCountH)
					{
					sourceIter.Reset(sourceRowAddress, sourceLeft);
					sourceCountH = sourceCountHReset;
					}

				if (not --matteCountH)
					{
					matteIter.Reset(matteRowAddress, matteLeft);
					matteCountH = matteCountHReset;
					}
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++sourceV;
		if (sourceV >= sourceBottom)
			sourceV = sourceTop;

 		++matteV;
		if (matteV >= matteBottom)
			matteV = matteTop;

		sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
		sourceIter.Reset(sourceRowAddress, sourceHStart);

		matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
		matteIter.Reset(matteRowAddress, matteHStart);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

template <class S, class M, class D, class O>
void sTile_SMDO_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied, const O& iOp)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int sourceLeft = iSourceF.left;
	int sourceWidth = W(iSourceF);
	int sourceHStart = iSourceF.left + sPositiveModulus(iSourceOrigin.h, sourceWidth);
	int sourceCountHStart = iSourceF.right - sourceHStart;
	int sourceCountHReset = sourceWidth;

	int sourceTop = iSourceF.top;
	int sourceBottom = iSourceF.bottom;
	int sourceHeight = H(iSourceF);
	int sourceVStart = iSourceF.top + sPositiveModulus(iSourceOrigin.v, sourceHeight);

	int sourceV = sourceVStart;

	int matteLeft = iMatteF.left;
	int matteWidth = W(iMatteF);
	int matteHStart = iMatteF.left + sPositiveModulus(iMatteOrigin.h, matteWidth);
	int matteCountHStart = iMatteF.right - matteHStart;
	int matteCountHReset = matteWidth;

	int matteTop = iMatteF.top;
	int matteBottom = iMatteF.bottom;
	int matteHeight = H(iMatteF);
	int matteVStart = iMatteF.top + sPositiveModulus(iMatteOrigin.v, matteHeight);

	int matteV = matteVStart;

	const void* sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sourceRowAddress,
		sourceHStart,
		iSourcePD);

	const void* matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		matteRowAddress,
		matteHStart,
		iMattePD);

	int currentDestV = iDestF.top;
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int sourceCountH = sourceCountHStart;
		int matteCountH = matteCountHStart;

		if (iSourcePremultiplied)
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();
				sAlpha(thePixel) = matteIter.ReadAlphaInc();
				RGBA destPixel = destIter.Read();
				iOp(thePixel, destPixel);
				destIter.WriteInc(destPixel);

				if (not --sourceCountH)
					{
					sourceIter.Reset(sourceRowAddress, sourceLeft);
					sourceCountH = sourceCountHReset;
					}

				if (not --matteCountH)
					{
					matteIter.Reset(matteRowAddress, matteLeft);
					matteCountH = matteCountHReset;
					}
				}
			}
		else
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();

				Comp alpha = matteIter.ReadAlphaInc();
				sRed(thePixel) *= alpha;
				sGreen(thePixel) *= alpha;
				sBlue(thePixel) *= alpha;
				sAlpha(thePixel) = alpha;

				RGBA destPixel = destIter.Read();

				iOp(thePixel, destPixel);

				destIter.WriteInc(destPixel);

				if (not --sourceCountH)
					{
					sourceIter.Reset(sourceRowAddress, sourceLeft);
					sourceCountH = sourceCountHReset;
					}

				if (not --matteCountH)
					{
					matteIter.Reset(matteRowAddress, matteLeft);
					matteCountH = matteCountHReset;
					}
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++sourceV;
		if (sourceV >= sourceBottom)
			sourceV = sourceTop;

 		++matteV;
		if (matteV >= matteBottom)
			matteV = matteTop;

		sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
		sourceIter.Reset(sourceRowAddress, sourceHStart);

		matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
		matteIter.Reset(matteRowAddress, matteHStart);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

// =================================================================================================
// MARK: - Copy variants

template <class S, class D>
void sCopy_SD_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD)
	{
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sCalcRowAddress(iSourceRD, iSource, iSourceStart.v),
		iSourceStart.h,
		iSourcePD);

	PixelIterW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			RGBA thePixel = sourceIter.ReadInc();
			destIter.WriteInc(thePixel);
			}

 		++row;
		if (row >= destHeight)
			break;

		sourceIter.Reset(
			sCalcRowAddress(iSourceRD, iSource, iSourceStart.v + row), iSourceStart.h);

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

template <class S, class D, class O>
void sCopy_SDO_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const O& iOp)
	{
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sCalcRowAddress(iSourceRD, iSource, iSourceStart.v),
		iSourceStart.h,
		iSourcePD);

	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			RGBA thePixel = sourceIter.ReadInc();
			RGBA destPixel = destIter.Read();
			iOp(thePixel, destPixel);
			destIter.WriteInc(destPixel);
			}

 		++row;
		if (row >= destHeight)
			break;

		sourceIter.Reset(
			sCalcRowAddress(iSourceRD, iSource, iSourceStart.v + row), iSourceStart.h);

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

template <class S, class M, class D>
void sCopy_SMD_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied)
	{
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sCalcRowAddress(iSourceRD, iSource, iSourceStart.v),
		iSourceStart.h,
		iSourcePD);

	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v),
		iMatteStart.h,
		iMattePD);

	PixelIterW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		if (iSourcePremultiplied)
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();
				sAlpha(thePixel) = matteIter.ReadAlphaInc();
				destIter.WriteInc(thePixel);
				}
			}
		else
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();

				Comp alpha = matteIter.ReadAlphaInc();
				sRed(thePixel) *= alpha;
				sGreen(thePixel) *= alpha;
				sBlue(thePixel) *= alpha;
				sAlpha(thePixel) = alpha;

				destIter.WriteInc(thePixel);
				}
			}

 		++row;
		if (row >= destHeight)
			break;

		sourceIter.Reset(
			sCalcRowAddress(iSourceRD, iSource, iSourceStart.v + row), iSourceStart.h);

		matteIter.Reset(
			sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v + row), iMatteStart.h);

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

template <class S, class M, class D, class O>
void sCopy_SMDO_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied,
	const O& iOp)
	{
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sCalcRowAddress(iSourceRD, iSource, iSourceStart.v),
		iSourceStart.h,
		iSourcePD);

	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v),
		iMatteStart.h,
		iMattePD);

	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		if (iSourcePremultiplied)
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();
				sAlpha(thePixel) = matteIter.ReadAlphaInc();
				RGBA destPixel = destIter.Read();
				iOp(thePixel, destPixel);
				destIter.WriteInc(destPixel);
				}
			}
		else
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();

				Comp alpha = matteIter.ReadAlphaInc();
				sRed(thePixel) *= alpha;
				sGreen(thePixel) *= alpha;
				sBlue(thePixel) *= alpha;
				sAlpha(thePixel) = alpha;

				RGBA destPixel = destIter.Read();

				iOp(thePixel, destPixel);

				destIter.WriteInc(destPixel);
				}
			}

 		++row;
		if (row >= destHeight)
			break;

		sourceIter.Reset(
			sCalcRowAddress(iSourceRD, iSource, iSourceStart.v + row), iSourceStart.h);

		matteIter.Reset(
			sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v + row), iMatteStart.h);

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

// =================================================================================================
// MARK: - Tile source, untiled matte

template <class S, class M, class D>
void sTileSource_SMD_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int sourceLeft = iSourceF.left;
	int sourceWidth = W(iSourceF);
	int sourceHStart = iSourceF.left + sPositiveModulus(iSourceOrigin.h, sourceWidth);
	int sourceCountHStart = iSourceF.right - sourceHStart;
	int sourceCountHReset = sourceWidth;

	int sourceTop = iSourceF.top;
	int sourceBottom = iSourceF.bottom;
	int sourceHeight = H(iSourceF);
	int sourceVStart = iSourceF.top + sPositiveModulus(iSourceOrigin.v, sourceHeight);

	int sourceV = sourceVStart;

	const void* sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sourceRowAddress,
		sourceHStart,
		iSourcePD);

	int matteV = iMatteStart.v;
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		sCalcRowAddress(iMatteRD, iMatte, matteV),
		iMatteStart.h,
		iMattePD);

	int currentDestV = iDestF.top;
	PixelIterW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int sourceCountH = sourceCountHStart;

		if (iSourcePremultiplied)
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();
				sAlpha(thePixel) = matteIter.ReadAlphaInc();
				destIter.WriteInc(thePixel);

				if (not --sourceCountH)
					{
					sourceIter.Reset(sourceRowAddress, sourceLeft);
					sourceCountH = sourceCountHReset;
					}
				}
			}
		else
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();

				Comp alpha = matteIter.ReadAlphaInc();
				sRed(thePixel) *= alpha;
				sGreen(thePixel) *= alpha;
				sBlue(thePixel) *= alpha;
				sAlpha(thePixel) = alpha;

				destIter.WriteInc(thePixel);

				if (not --sourceCountH)
					{
					sourceIter.Reset(sourceRowAddress, sourceLeft);
					sourceCountH = sourceCountHReset;
					}
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++sourceV;
		if (sourceV >= sourceBottom)
			sourceV = sourceTop;

 		++matteV;

		sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
		sourceIter.Reset(sourceRowAddress, sourceHStart);

		matteIter.Reset(sCalcRowAddress(iMatteRD, iMatte, matteV), matteV);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

template <class S, class M, class D, class O>
void sTileSource_SMDO_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied,
	const O& iOp)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int sourceLeft = iSourceF.left;
	int sourceWidth = W(iSourceF);
	int sourceHStart = iSourceF.left + sPositiveModulus(iSourceOrigin.h, sourceWidth);
	int sourceCountHStart = iSourceF.right - sourceHStart;
	int sourceCountHReset = sourceWidth;

	int sourceTop = iSourceF.top;
	int sourceBottom = iSourceF.bottom;
	int sourceHeight = H(iSourceF);
	int sourceVStart = iSourceF.top + sPositiveModulus(iSourceOrigin.v, sourceHeight);

	int sourceV = sourceVStart;

	const void* sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sourceRowAddress,
		sourceHStart,
		iSourcePD);

	int matteV = iMatteStart.v;
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		sCalcRowAddress(iMatteRD, iMatte, matteV),
		iMatteStart.h,
		iMattePD);

	int currentDestV = iDestF.top;
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int sourceCountH = sourceCountHStart;

		if (iSourcePremultiplied)
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();
				sAlpha(thePixel) = matteIter.ReadAlphaInc();
				RGBA destPixel = destIter.Read();
				iOp(thePixel, destPixel);
				destIter.WriteInc(destPixel);

				if (not --sourceCountH)
					{
					sourceIter.Reset(sourceRowAddress, sourceLeft);
					sourceCountH = sourceCountHReset;
					}
				}
			}
		else
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();

				Comp alpha = matteIter.ReadAlphaInc();
				sRed(thePixel) *= alpha;
				sGreen(thePixel) *= alpha;
				sBlue(thePixel) *= alpha;
				sAlpha(thePixel) = alpha;

				RGBA destPixel = destIter.Read();

				iOp(thePixel, destPixel);

				destIter.WriteInc(destPixel);

				if (not --sourceCountH)
					{
					sourceIter.Reset(sourceRowAddress, sourceLeft);
					sourceCountH = sourceCountHReset;
					}
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++sourceV;
		if (sourceV >= sourceBottom)
			sourceV = sourceTop;

 		++matteV;

		sourceRowAddress = sCalcRowAddress(iSourceRD, iSource, sourceV);
		sourceIter.Reset(sourceRowAddress, sourceHStart);

		matteIter.Reset(sCalcRowAddress(iMatteRD, iMatte, matteV), matteV);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

// =================================================================================================
// MARK: - Tile matte, untiled source

template <class S, class M, class D>
void sTileMatte_SMD_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int matteLeft = iMatteF.left;
	int matteWidth = W(iMatteF);
	int matteHStart = iMatteF.left + sPositiveModulus(iMatteOrigin.h, matteWidth);
	int matteCountHStart = iMatteF.right - matteHStart;
	int matteCountHReset = matteWidth;

	int matteTop = iMatteF.top;
	int matteBottom = iMatteF.bottom;
	int matteHeight = H(iMatteF);
	int matteVStart = iMatteF.top + sPositiveModulus(iMatteOrigin.v, matteHeight);

	int sourceV = iSourceStart.v;
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sCalcRowAddress(iSourceRD, iSource, sourceV),
		iSourceStart.h,
		iSourcePD);

	int matteV = matteVStart;
	const void* matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		matteRowAddress,
		matteHStart,
		iMattePD);

	int currentDestV = iDestF.top;
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int matteCountH = matteCountHStart;

		if (iSourcePremultiplied)
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();
				sAlpha(thePixel) = matteIter.ReadAlphaInc();
				destIter.WriteInc(thePixel);

				if (not --matteCountH)
					{
					matteIter.Reset(matteRowAddress, matteLeft);
					matteCountH = matteCountHReset;
					}
				}
			}
		else
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();

				Comp alpha = matteIter.ReadAlphaInc();
				sRed(thePixel) *= alpha;
				sGreen(thePixel) *= alpha;
				sBlue(thePixel) *= alpha;
				sAlpha(thePixel) = alpha;

				destIter.WriteInc(thePixel);

				if (not --matteCountH)
					{
					matteIter.Reset(matteRowAddress, matteLeft);
					matteCountH = matteCountHReset;
					}
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++sourceV;

 		++matteV;
		if (matteV >= matteBottom)
			matteV = matteTop;

		sourceIter.Reset(sCalcRowAddress(iSourceRD, iSource, sourceV), iSourceStart.h);

		matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
		matteIter.Reset(matteRowAddress, matteHStart);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

// Trinary with operation
template <class S, class M, class D, class O>
void sTileMatte_SMDO_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied, const O& iOp)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int matteLeft = iMatteF.left;
	int matteWidth = W(iMatteF);
	int matteHStart = iMatteF.left + sPositiveModulus(iMatteOrigin.h, matteWidth);
	int matteCountHStart = iMatteF.right - matteHStart;
	int matteCountHReset = matteWidth;

	int matteTop = iMatteF.top;
	int matteBottom = iMatteF.bottom;
	int matteHeight = H(iMatteF);
	int matteVStart = iMatteF.top + sPositiveModulus(iMatteOrigin.v, matteHeight);

	int sourceV = iSourceStart.v;
	PixelIterR_T<S> sourceIter(
		iSourceRD.fPixvalDesc,
		sCalcRowAddress(iSourceRD, iSource, sourceV),
		iSourceStart.h,
		iSourcePD);

	int matteV = matteVStart;
	const void* matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		matteRowAddress,
		matteHStart,
		iMattePD);

	int currentDestV = iDestF.top;
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int matteCountH = matteCountHStart;

		if (iSourcePremultiplied)
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();
				sAlpha(thePixel) = matteIter.ReadAlphaInc();
				RGBA destPixel = destIter.Read();
				iOp(thePixel, destPixel);
				destIter.WriteInc(destPixel);

				if (not --matteCountH)
					{
					matteIter.Reset(matteRowAddress, matteLeft);
					matteCountH = matteCountHReset;
					}
				}
			}
		else
			{
			while (--destCountH)
				{
				RGBA thePixel = sourceIter.ReadInc();

				Comp alpha = matteIter.ReadAlphaInc();
				sRed(thePixel) *= alpha;
				sGreen(thePixel) *= alpha;
				sBlue(thePixel) *= alpha;
				sAlpha(thePixel) = alpha;

				RGBA destPixel = destIter.Read();

				iOp(thePixel, destPixel);

				destIter.WriteInc(destPixel);

				if (not --matteCountH)
					{
					matteIter.Reset(matteRowAddress, matteLeft);
					matteCountH = matteCountHReset;
					}
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++sourceV;

 		++matteV;
		if (matteV >= matteBottom)
			matteV = matteTop;

		sourceIter.Reset(sCalcRowAddress(iSourceRD, iSource, sourceV), iSourceStart.h);

		matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
		matteIter.Reset(matteRowAddress, matteHStart);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

// =================================================================================================
// MARK: - Fill/color variants

static void sFillPixval(const RD& iDestRD, void* oDest, const RectPOD& iDestF,
			uint32 iPixval)
	{
	PixvalIterW destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			destIter.WriteInc(iPixval);

 		++row;
		if (row >= destHeight)
			break;

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

template <class D, class O>
void sColor_DO_T(
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const RGBA& iColor,
	const O& iOp)
	{
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			RGBA destPixel = destIter.Read();
			iOp(iColor, destPixel);
			destIter.WriteInc(destPixel);
			}

 		++row;
		if (row >= destHeight)
			break;

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

template <class M, class D>
void sColor_MD_T(
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const RGBA& iColor)
	{
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v),
		iMatteStart.h,
		iMattePD);

	PixelIterW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			Comp alpha = matteIter.ReadAlphaInc();
			destIter.WriteInc(iColor * alpha);
			}

 		++row;
		if (row >= destHeight)
			break;

		matteIter.Reset(
			sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v + row), iMatteStart.h);

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

template <class M, class D, class O>
void sColor_MDO_T(
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const RGBA& iColor,
	const O& iOp)
	{
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v),
		iMatteStart.h,
		iMattePD);

	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			Comp alpha = matteIter.ReadAlphaInc();
			RGBA destPixel = destIter.Read();
			iOp(iColor * alpha, destPixel);
			destIter.WriteInc(destPixel);
			}

 		++row;
		if (row >= destHeight)
			break;

		matteIter.Reset(
			sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v + row), iMatteStart.h);

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

template <class M, class D>
void sColorTile_MD_T(
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const RGBA& iColor)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int matteLeft = iMatteF.left;
	int matteWidth = W(iMatteF);
	int matteHStart = iMatteF.left + sPositiveModulus(iMatteOrigin.h, matteWidth);
	int matteCountHStart = iMatteF.right - matteHStart;
	int matteCountHReset = matteWidth;

	int matteTop = iMatteF.top;
	int matteBottom = iMatteF.bottom;
	int matteHeight = H(iMatteF);
	int matteVStart = iMatteF.top + sPositiveModulus(iMatteOrigin.v, matteHeight);

	int matteV = matteVStart;
	const void* matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		matteRowAddress,
		matteHStart,
		iMattePD);

	int currentDestV = iDestF.top;
	PixelIterW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int matteCountH = matteCountHStart;

		while (--destCountH)
			{
			Comp alpha = matteIter.ReadAlphaInc();
			destIter.WriteInc(iColor * alpha);

			if (not --matteCountH)
				{
				matteIter.Reset(matteRowAddress, matteLeft);
				matteCountH = matteCountHReset;
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++matteV;
		if (matteV >= matteBottom)
			matteV = matteTop;

		matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
		matteIter.Reset(matteRowAddress, matteHStart);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

template <class M, class D, class O>
void sColorTile_MDO_T(
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const RGBA& iColor, const O& iOp)
	{
	int destWidth = W(iDestF);
	int destBottom = iDestF.bottom;

	int matteLeft = iMatteF.left;
	int matteWidth = W(iMatteF);
	int matteHStart = iMatteF.left + sPositiveModulus(iMatteOrigin.h, matteWidth);
	int matteCountHStart = iMatteF.right - matteHStart;
	int matteCountHReset = matteWidth;

	int matteTop = iMatteF.top;
	int matteBottom = iMatteF.bottom;
	int matteHeight = H(iMatteF);
	int matteVStart = iMatteF.top + sPositiveModulus(iMatteOrigin.v, matteHeight);

	int matteV = matteVStart;
	const void* matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		matteRowAddress,
		matteHStart,
		iMattePD);

	int currentDestV = iDestF.top;
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, currentDestV),
		iDestF.left,
		iDestPD);

	for (;;)
		{
		int destCountH = destWidth + 1;
		int matteCountH = matteCountHStart;

		while (--destCountH)
			{
			Comp alpha = matteIter.ReadAlphaInc();
			RGBA destPixel = destIter.Read();
			iOp(iColor * alpha, destPixel);
			destIter.WriteInc(destPixel);

			if (not --matteCountH)
				{
				matteIter.Reset(matteRowAddress, matteLeft);
				matteCountH = matteCountHReset;
				}
			}

 		++currentDestV;
		if (currentDestV >= destBottom)
			break;

 		++matteV;
		if (matteV >= matteBottom)
			matteV = matteTop;

		matteRowAddress = sCalcRowAddress(iMatteRD, iMatte, matteV);
		matteIter.Reset(matteRowAddress, matteHStart);

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, currentDestV), iDestF.left);
		}
	}

// =================================================================================================
// MARK: - Blit dispatchers

// Replicate iSourceF over iDestF, aligning iSourceOrigin with iDestF.TopLeft()
template <class S, class D>
void sBlit_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sTile_SD_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iDestRD, oDest, iDestF, iDestPD);
			break;
			}
		case eOp_Over:
			{
			sTile_SDO_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sTile_SDO_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				Compose_Plus());
			break;
			}
		}
	}

// Copy source to iDestF without replication. Actual drawn rectangle will be smaller
// than iDestF if iSourceF is smaller.
template <class S, class D>
void sBlit_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sCopy_SD_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iDestRD, oDest, iDestF, iDestPD);
			break;
			}
		case eOp_Over:
			{
			sCopy_SDO_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iDestRD, oDest, iDestF, iDestPD,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sCopy_SDO_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iDestRD, oDest, iDestF, iDestPD,
				Compose_Plus());
			break;
			}
		}
	}

// Replicate iSourceF masked by replicated iMatteF.
template <class S, class M, class D>
void sBlit_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied, EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sTile_SMD_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied);
			break;
			}
		case eOp_Over:
			{
			sTile_SMDO_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sTile_SMDO_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied,
				Compose_Plus());
			break;
			}
		}
	}

// Replicate iSourceF masked by iMatteF.
template <class S, class M, class D>
void sBlit_T(
	const RD& iSourceRD, const void* iSource, const RectPOD& iSourceF, const S& iSourcePD,
	PointPOD iSourceOrigin,
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied, EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sTileSource_SMD_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied);
			break;
			}
		case eOp_Over:
			{
			sTileSource_SMDO_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sTileSource_SMDO_T(
				iSourceRD, iSource, iSourceF, iSourcePD,
				iSourceOrigin,
				iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied,
				Compose_Plus());
			break;
			}
		}
	}

// Draw iSourceF masked by replicated iMatteF.
template <class S, class M, class D>
void sBlit_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied, EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sTileMatte_SMD_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied);
			break;
			}
		case eOp_Over:
			{
			sTileMatte_SMDO_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sTileMatte_SMDO_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied,
				Compose_Plus());
			break;
			}
		}
	}

// Draw iSourceF masked by iMatteF into iDestF.
template <class S, class M, class D>
void sBlit_T(
	const RD& iSourceRD, const void* iSource, const S& iSourcePD,
	PointPOD iSourceStart,
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	bool iSourcePremultiplied, EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sCopy_SMD_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied);
			break;
			}
		case eOp_Over:
			{
			sCopy_SMDO_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sCopy_SMDO_T(
				iSourceRD, iSource, iSourcePD,
				iSourceStart,
				iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iSourcePremultiplied,
				Compose_Plus());
			break;
			}
		}
	}

// Fill iDestF with iColor
template <class D>
void sColor_T(
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const RGBA& iColor,
	EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sFillPixval(iDestRD, oDest, iDestF, iDestPD.AsPixval(iColor));
			break;
			}
		case eOp_Over:
			{
			sColor_DO_T(iDestRD, oDest, iDestF, iDestPD,
				iColor,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sColor_DO_T(iDestRD, oDest, iDestF, iDestPD,
				iColor,
				Compose_Plus());
			break;
			}
		}
	}

// Fill iDestF matted
template <class M, class D>
void sColor_T(
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const RGBA& iColor,
	EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sColor_MD_T(iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iColor);
			break;
			}
		case eOp_Over:
			{
			sColor_MDO_T(iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iColor,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sColor_MDO_T(iMatteRD, iMatte, iMattePD,
				iMatteStart,
				iDestRD, oDest, iDestF, iDestPD,
				iColor,
				Compose_Plus());
			break;
			}
		}
	}

// Fill iDestbounds with tiled matte
template <class M, class D>
void sColorTile_T(
	const RD& iMatteRD, const void* iMatte, const RectPOD& iMatteF, const M& iMattePD,
	PointPOD iMatteOrigin,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD,
	const RGBA& iColor,
	EOp iOp)
	{
	switch (iOp)
		{
		default:
		case eOp_Copy:
			{
			sColorTile_MD_T(iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iColor);
			break;
			}
		case eOp_Over:
			{
			sColorTile_MDO_T(iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iColor,
				Compose_Over());
			break;
			}
		case eOp_Plus:
			{
			sColorTile_MDO_T(iMatteRD, iMatte, iMatteF, iMattePD,
				iMatteOrigin,
				iDestRD, oDest, iDestF, iDestPD,
				iColor,
				Compose_Plus());
			break;
			}
		}
	}

// Invert iDestbounds
template <class D>
void sInvert_T(const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD)
	{
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			RGBA destPixel = destIter.Read();
			sRed(destPixel) = sAlpha(destPixel) - sRed(destPixel);
			sGreen(destPixel) = sAlpha(destPixel) - sGreen(destPixel);
			sBlue(destPixel) = sAlpha(destPixel) - sBlue(destPixel);
			destIter.WriteInc(destPixel);
			}

 		++row;
		if (row >= destHeight)
			break;

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

// Multiply r,g, b & alpha by iAmount
template <class D>
void sOpaque_T(
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD, Comp iAmount)
	{
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			RGBA destPixel = destIter.Read();
			destIter.WriteInc(destPixel * iAmount);
			}

 		++row;
		if (row >= destHeight)
			break;

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

// Multiply r, g, b by iAmount, leaving alpha alone.
template <class D>
void sDarken_T(
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD, Comp iAmount)
	{
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			RGBA destPixel = destIter.Read();
			sRed(destPixel) *= iAmount;
			sGreen(destPixel) *= iAmount;
			sBlue(destPixel) *= iAmount;
			destIter.WriteInc(destPixel);
			}

 		++row;
		if (row >= destHeight)
			break;

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

// Multiply alpha by iAmount, leaving r,g,b alone
template <class D>
void sFade_T(
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD, Comp iAmount)
	{
	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			RGBA destPixel = destIter.Read();
			sAlpha(destPixel) *= iAmount;
			destIter.WriteInc(destPixel);
			}

 		++row;
		if (row >= destHeight)
			break;

		destIter.Reset(sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

// Apply the alpha values in matte to dest
template <class M, class D>
void sApplyMatte_T(
	const RD& iMatteRD, const void* iMatte, const M& iMattePD,
	PointPOD iMatteStart,
	const RD& iDestRD, void* oDest, const RectPOD& iDestF, const D& iDestPD)
	{
	PixelIterR_T<M> matteIter(
		iMatteRD.fPixvalDesc,
		sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v),
		iMatteStart.h,
		iMattePD);

	PixelIterRW_T<D> destIter(
		iDestRD.fPixvalDesc,
		sCalcRowAddress(iDestRD, oDest, iDestF.top),
		iDestF.left,
		iDestPD);

	int destWidth = W(iDestF);
	int destHeight = H(iDestF);
	int row = 0;
	for (;;)
		{
		int destCountH = destWidth + 1;

		while (--destCountH)
			{
			RGBA thePixel = destIter.Read() * matteIter.ReadAlphaInc();
			destIter.WriteInc(thePixel);
			}

 		++row;
		if (row >= destHeight)
			break;

		matteIter.Reset(
			sCalcRowAddress(iMatteRD, iMatte, iMatteStart.v + row), iMatteStart.h);

		destIter.Reset(
			sCalcRowAddress(iDestRD, oDest, iDestF.top + row), iDestF.left);
		}
	}

} // namespace Pixels
} // namespace ZooLib

#endif // __ZooLib_Pixels_BlitPriv_h__
