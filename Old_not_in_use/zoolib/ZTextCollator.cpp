// Copyright (c) 2003 Andrew Green and Learning in Motion, Inc.
// MIT License. http://www.zoolib.org

#include "zoolib/Unicode.h"

#include "zoolib/ZTextCollator.h"

namespace ZooLib {

// =================================================================================================
#pragma mark - Factories

typedef FunctionChain<ZRef<ZTextCollatorRep>, const ZTextCollatorRep::Param_t&> Factory_t;

// =================================================================================================
/**
\defgroup TextCollation Text Collation

Text collation is the act of placing strings in a sequence acccording to some set of rules.
Collation is transitive, so that A&nbsp;<&nbsp;B and B&nbsp;<&nbsp;C implies A&nbsp;<&nbsp;C,
so collation must operate consistently against arbitrary pairs of strings.

Text collation is obviously useful for the presentation of lists of strings in a sequence that
is meaningful and predictable to an end user. It is also useful in determining equivalence between
pieces of text, with account taken of cultural expectations. The act of comparing one string
against another is the basis for searching a target string for occurrences of a pattern string.

Collation is not applied to individual code points, but to strings, so that context-based rules
have a context in which to operate.

Instances of ZTextCollator provide text collation facilities to ZooLib and applications.
*/

// =================================================================================================
#pragma mark - ZTextCollator

/**
\class ZTextCollator
\ingroup TextCollation

ZTextCollator is a \link RefCount smart pointer\endlink containing an instance of a concrete
subclass of ZTextCollatorRep. It's initialized with a strength and optionally a locale name,
with which it initializes an instance of an appropriate subclass of ZTextCollatorRep.

The strength indicates which features of code points and code point clusters are to be considered
in comparison operations. The locale name determines which suite of rules are to be applied in
making comparisons. If no locale name is provided then the universal locale is assumed, which has
no language, culture or domain-specific rules and is useful when the text to be operated on is of
unknown or mixed provenance.

Every code point has a unique identity, and also has other features associated with it. For code
points in the latinate ranges these features include their case and diacritics. Other ranges have
other kinds of features.

The strength parameter indicates which features are to be taken into account when making
comparisons. The weakest comparison, where the fewest features are to be taken into account
is strength 1. Larger values indicate that more features are significant. The value is passed to
the underlying implementation unchanged, so to portably indicate the strongest possible
comparison we use strength 0.

Currently there are two concrete subclasses of ZTextCollatorRep. ZTextCollatorRep_ASCII is a
minimalist implementation that understands only the attributes of the ASCII range, and thus
needn't rely on any external API or data. ZTextCollatorRep_ICU uses ICU and thus correctly
handles all code points, and has a comprehensive suite of collation rules.

For more information on ICU collation see
<a href="http://icu.sourceforge.net/userguide/Collate_Intro.html">Collation Introduction</a>.
*/

ZTextCollator::ZTextCollator()
	{}

ZTextCollator::ZTextCollator(const ZTextCollator& iOther)
:	fRep(iOther.fRep)
	{}

ZTextCollator::~ZTextCollator()
	{}

ZTextCollator& ZTextCollator::operator=(const ZTextCollator& iOther)
	{
	fRep = iOther.fRep;
	return *this;
	}

ZTextCollator::ZTextCollator(int iStrength)
	{
	ZTextCollatorRep::Param_t theParam;
	theParam.fStrength = iStrength;
	fRep = Factory_t::sInvoke(theParam);
	}

ZTextCollator::ZTextCollator(const string8& iLocaleName, int iStrength)
	{
	ZTextCollatorRep::Param_t theParam;
	theParam.fLocaleName = iLocaleName;
	theParam.fStrength = iStrength;
	fRep = Factory_t::sInvoke(theParam);
	}

ZTextCollator::ZTextCollator(const char* iLocaleName, int iStrength)
	{
	ZTextCollatorRep::Param_t theParam;
	theParam.fLocaleName = iLocaleName;
	theParam.fStrength = iStrength;
	fRep = Factory_t::sInvoke(theParam);
	}

bool ZTextCollator::Equals(const string8& iLeft, const string8& iRight) const
	{
	this->pMakeSafe();
	return fRep->Equals(iLeft, iRight);
	}

bool ZTextCollator::Equals(const string16& iLeft, const string16& iRight) const
	{
	this->pMakeSafe();
	return fRep->Equals(iLeft, iRight);
	}

bool ZTextCollator::Equals(const string32& iLeft, const string32& iRight) const
	{
	this->pMakeSafe();
	return fRep->Equals(iLeft, iRight);
	}

bool ZTextCollator::Equals(const UTF8* iLeft, size_t iLeftLength,
	const UTF8* iRight, size_t iRightLength) const
	{
	this->pMakeSafe();
	return fRep->Equals(iLeft, iLeftLength, iRight, iRightLength);
	}

bool ZTextCollator::Equals(const UTF16* iLeft, size_t iLeftLength,
	const UTF16* iRight, size_t iRightLength) const
	{
	this->pMakeSafe();
	return fRep->Equals(iLeft, iLeftLength, iRight, iRightLength);
	}

bool ZTextCollator::Equals(const UTF32* iLeft, size_t iLeftLength,
	const UTF32* iRight, size_t iRightLength) const
	{
	this->pMakeSafe();
	return fRep->Equals(iLeft, iLeftLength, iRight, iRightLength);
	}

int ZTextCollator::Compare(const string8& iLeft, const string8& iRight) const
	{
	this->pMakeSafe();
	return fRep->Compare(iLeft, iRight);
	}

int ZTextCollator::Compare(const string16& iLeft, const string16& iRight) const
	{
	this->pMakeSafe();
	return fRep->Compare(iLeft, iRight);
	}

int ZTextCollator::Compare(const string32& iLeft, const string32& iRight) const
	{
	this->pMakeSafe();
	return fRep->Compare(iLeft, iRight);
	}

int ZTextCollator::Compare(const UTF8* iLeft, size_t iLeftLength,
	const UTF8* iRight, size_t iRightLength) const
	{
	this->pMakeSafe();
	return fRep->Compare(iLeft, iLeftLength, iRight, iRightLength);
	}

int ZTextCollator::Compare(const UTF16* iLeft, size_t iLeftLength,
	const UTF16* iRight, size_t iRightLength) const
	{
	this->pMakeSafe();
	return fRep->Compare(iLeft, iLeftLength, iRight, iRightLength);
	}

int ZTextCollator::Compare(const UTF32* iLeft, size_t iLeftLength,
	const UTF32* iRight, size_t iRightLength) const
	{
	this->pMakeSafe();
	return fRep->Compare(iLeft, iLeftLength, iRight, iRightLength);
	}

bool ZTextCollator::Contains(const string8& iPattern, const string8& iTarget) const
	{
	this->pMakeSafe();
	return fRep->Contains(iPattern, iTarget);
	}

bool ZTextCollator::Contains(const string16& iPattern, const string16& iTarget) const
	{
	this->pMakeSafe();
	return fRep->Contains(iPattern, iTarget);
	}

bool ZTextCollator::Contains(const string32& iPattern, const string32& iTarget) const
	{
	this->pMakeSafe();
	return fRep->Contains(iPattern, iTarget);
	}

bool ZTextCollator::Contains(const UTF8* iPattern, size_t iPatternLength,
	const UTF8* iTarget, size_t iTargetLength) const
	{
	this->pMakeSafe();
	return fRep->Contains(iPattern, iPatternLength, iTarget, iTargetLength);
	}

bool ZTextCollator::Contains(const UTF16* iPattern, size_t iPatternLength,
	const UTF16* iTarget, size_t iTargetLength) const
	{
	this->pMakeSafe();
	return fRep->Contains(iPattern, iPatternLength, iTarget, iTargetLength);
	}

bool ZTextCollator::Contains(const UTF32* iPattern, size_t iPatternLength,
	const UTF32* iTarget, size_t iTargetLength) const
	{
	this->pMakeSafe();
	return fRep->Contains(iPattern, iPatternLength, iTarget, iTargetLength);
	}

void ZTextCollator::pMakeSafe() const
	{
	if (fRep->IsShared())
		{
		ZRef<ZTextCollatorRep> newRep = fRep->SafeCopy();
		const_cast<ZTextCollator*>(this)->fRep = newRep;
		}
	}

// =================================================================================================
#pragma mark - ZTextCollatorRep

ZTextCollatorRep::ZTextCollatorRep()
	{}

ZTextCollatorRep::ZTextCollatorRep(const ZTextCollatorRep& iOther)
	{}

ZTextCollatorRep& ZTextCollatorRep::operator=(const ZTextCollatorRep& iOther)
	{ return *this;
	}

ZTextCollatorRep::~ZTextCollatorRep()
	{}

/**
Return an instance of ZTextCollatorRep that is either reentrant or is an independent
and thus thread-safe copy of the current instance. By default we simply call clone,
reentrant collators should override this method and simply return 'this'.
*/
ZRef<ZTextCollatorRep> ZTextCollatorRep::SafeCopy()
	{ return this->Clone(); }

// =================================================================================================
#pragma mark - ZTextCollatorRep_NativeUTF8

bool ZTextCollatorRep_NativeUTF8::Equals(const string16& iLeft, const string16& iRight)
	{ return this->Equals(Unicode::sAsUTF8(iLeft), Unicode::sAsUTF8(iRight)); }

bool ZTextCollatorRep_NativeUTF8::Equals(const string32& iLeft, const string32& iRight)
	{ return this->Equals(Unicode::sAsUTF8(iLeft), Unicode::sAsUTF8(iRight)); }

bool ZTextCollatorRep_NativeUTF8::Equals(const UTF16* iLeft, size_t iLeftLength,
	const UTF16* iRight, size_t iRightLength)
	{
	return this->Equals(Unicode::sAsUTF8(iLeft, iLeftLength),
		Unicode::sAsUTF8(iRight, iRightLength));
	}

bool ZTextCollatorRep_NativeUTF8::Equals(const UTF32* iLeft, size_t iLeftLength,
	const UTF32* iRight, size_t iRightLength)
	{
	return this->Equals(Unicode::sAsUTF8(iLeft, iLeftLength),
		Unicode::sAsUTF8(iRight, iRightLength));
	}

int ZTextCollatorRep_NativeUTF8::Compare(const string16& iLeft, const string16& iRight)
	{ return this->Compare(Unicode::sAsUTF8(iLeft), Unicode::sAsUTF8(iRight)); }

int ZTextCollatorRep_NativeUTF8::Compare(const string32& iLeft, const string32& iRight)
	{ return this->Compare(Unicode::sAsUTF8(iLeft), Unicode::sAsUTF8(iRight)); }

int ZTextCollatorRep_NativeUTF8::Compare(const UTF16* iLeft, size_t iLeftLength,
	const UTF16* iRight, size_t iRightLength)
	{
	return this->Compare(Unicode::sAsUTF8(iLeft, iLeftLength),
		Unicode::sAsUTF8(iRight, iRightLength));
	}

int ZTextCollatorRep_NativeUTF8::Compare(const UTF32* iLeft, size_t iLeftLength,
	const UTF32* iRight, size_t iRightLength)
	{
	return this->Compare(Unicode::sAsUTF8(iLeft, iLeftLength),
		Unicode::sAsUTF8(iRight, iRightLength));
	}

bool ZTextCollatorRep_NativeUTF8::Contains(const string16& iPattern, const string16& iTarget)
	{ return this->Contains(Unicode::sAsUTF8(iPattern), Unicode::sAsUTF8(iTarget)); }

bool ZTextCollatorRep_NativeUTF8::Contains(const string32& iPattern, const string32& iTarget)
	{ return this->Contains(Unicode::sAsUTF8(iPattern), Unicode::sAsUTF8(iTarget)); }

bool ZTextCollatorRep_NativeUTF8::Contains(const UTF16* iPattern, size_t iPatternLength,
	const UTF16* iTarget, size_t iTargetLength)
	{
	return this->Contains(Unicode::sAsUTF8(iPattern, iPatternLength),
		Unicode::sAsUTF8(iTarget, iTargetLength));
	}

bool ZTextCollatorRep_NativeUTF8::Contains(const UTF32* iPattern, size_t iPatternLength,
	const UTF32* iTarget, size_t iTargetLength)
	{
	return this->Contains(Unicode::sAsUTF8(iPattern, iPatternLength),
		Unicode::sAsUTF8(iTarget, iTargetLength));
	}

// =================================================================================================
#pragma mark - ZTextCollatorRep_NativeUTF16

bool ZTextCollatorRep_NativeUTF16::Equals(const string8& iPattern, const string8& iRight)
	{ return this->Equals(Unicode::sAsUTF16(iPattern), Unicode::sAsUTF16(iRight)); }

bool ZTextCollatorRep_NativeUTF16::Equals(const string32& iPattern, const string32& iRight)
	{ return this->Equals(Unicode::sAsUTF16(iPattern), Unicode::sAsUTF16(iRight)); }

bool ZTextCollatorRep_NativeUTF16::Equals(const UTF8* iLeft, size_t iLeftLength,
	const UTF8* iRight, size_t iRightLength)
	{
	return this->Equals(Unicode::sAsUTF16(iLeft, iLeftLength),
		Unicode::sAsUTF16(iRight, iRightLength));
	}

bool ZTextCollatorRep_NativeUTF16::Equals(const UTF32* iLeft, size_t iLeftLength,
	const UTF32* iRight, size_t iRightLength)
	{
	return this->Equals(Unicode::sAsUTF16(iLeft, iLeftLength),
		Unicode::sAsUTF16(iRight, iRightLength));
	}

int ZTextCollatorRep_NativeUTF16::Compare(const string8& iLeft, const string8& iRight)
	{ return this->Compare(Unicode::sAsUTF16(iLeft), Unicode::sAsUTF16(iRight)); }

int ZTextCollatorRep_NativeUTF16::Compare(const string32& iLeft, const string32& iRight)
	{ return this->Compare(Unicode::sAsUTF16(iLeft), Unicode::sAsUTF16(iRight)); }

int ZTextCollatorRep_NativeUTF16::Compare(const UTF8* iLeft, size_t iLeftLength,
	const UTF8* iRight, size_t iRightLength)
	{
	return this->Compare(Unicode::sAsUTF16(iLeft, iLeftLength),
		Unicode::sAsUTF16(iRight, iRightLength));
	}

int ZTextCollatorRep_NativeUTF16::Compare(const UTF32* iLeft, size_t iLeftLength,
	const UTF32* iRight, size_t iRightLength)
	{
	return this->Compare(Unicode::sAsUTF16(iLeft, iLeftLength),
		Unicode::sAsUTF16(iRight, iRightLength));
	}

bool ZTextCollatorRep_NativeUTF16::Contains(const string8& iPattern, const string8& iTarget)
	{ return this->Contains(Unicode::sAsUTF16(iPattern), Unicode::sAsUTF16(iTarget)); }

bool ZTextCollatorRep_NativeUTF16::Contains(const string32& iPattern, const string32& iTarget)
	{ return this->Contains(Unicode::sAsUTF16(iPattern), Unicode::sAsUTF16(iTarget)); }

bool ZTextCollatorRep_NativeUTF16::Contains(const UTF8* iPattern, size_t iPatternLength,
	const UTF8* iTarget, size_t iTargetLength)
	{
	return this->Contains(Unicode::sAsUTF16(iPattern, iPatternLength),
		Unicode::sAsUTF16(iTarget, iTargetLength));
	}

bool ZTextCollatorRep_NativeUTF16::Contains(const UTF32* iPattern, size_t iPatternLength,
	const UTF32* iTarget, size_t iTargetLength)
	{
	return this->Contains(Unicode::sAsUTF16(iPattern, iPatternLength),
		Unicode::sAsUTF16(iTarget, iTargetLength));
	}

// =================================================================================================
#pragma mark - ZTextCollatorRep_NativeUTF32

bool ZTextCollatorRep_NativeUTF32::Equals(const string8& iPattern, const string8& iRight)
	{ return this->Equals(Unicode::sAsUTF32(iPattern), Unicode::sAsUTF32(iRight)); }

bool ZTextCollatorRep_NativeUTF32::Equals(const string16& iPattern, const string16& iRight)
	{ return this->Equals(Unicode::sAsUTF32(iPattern), Unicode::sAsUTF32(iRight)); }

bool ZTextCollatorRep_NativeUTF32::Equals(const UTF8* iLeft, size_t iLeftLength,
	const UTF8* iRight, size_t iRightLength)
	{
	return this->Equals(Unicode::sAsUTF32(iLeft, iLeftLength),
		Unicode::sAsUTF32(iRight, iRightLength));
	}

bool ZTextCollatorRep_NativeUTF32::Equals(const UTF16* iLeft, size_t iLeftLength,
	const UTF16* iRight, size_t iRightLength)
	{
	return this->Equals(Unicode::sAsUTF32(iLeft, iLeftLength),
		Unicode::sAsUTF32(iRight, iRightLength));
	}

int ZTextCollatorRep_NativeUTF32::Compare(const string8& iLeft, const string8& iRight)
	{ return this->Compare(Unicode::sAsUTF32(iLeft), Unicode::sAsUTF32(iRight)); }

int ZTextCollatorRep_NativeUTF32::Compare(const string16& iLeft, const string16& iRight)
	{ return this->Compare(Unicode::sAsUTF32(iLeft), Unicode::sAsUTF32(iRight)); }

int ZTextCollatorRep_NativeUTF32::Compare(const UTF8* iLeft, size_t iLeftLength,
	const UTF8* iRight, size_t iRightLength)
	{
	return this->Compare(Unicode::sAsUTF32(iLeft, iLeftLength),
		Unicode::sAsUTF32(iRight, iRightLength));
	}

int ZTextCollatorRep_NativeUTF32::Compare(const UTF16* iLeft, size_t iLeftLength,
	const UTF16* iRight, size_t iRightLength)
	{
	return this->Compare(Unicode::sAsUTF32(iLeft, iLeftLength),
		Unicode::sAsUTF32(iRight, iRightLength));
	}

bool ZTextCollatorRep_NativeUTF32::Contains(const string8& iPattern, const string8& iTarget)
	{ return this->Contains(Unicode::sAsUTF32(iPattern), Unicode::sAsUTF32(iTarget)); }

bool ZTextCollatorRep_NativeUTF32::Contains(const string16& iPattern, const string16& iTarget)
	{ return this->Contains(Unicode::sAsUTF32(iPattern), Unicode::sAsUTF32(iTarget)); }

bool ZTextCollatorRep_NativeUTF32::Contains(const UTF8* iPattern, size_t iPatternLength,
	const UTF8* iTarget, size_t iTargetLength)
	{
	return this->Contains(Unicode::sAsUTF32(iPattern, iPatternLength),
		Unicode::sAsUTF32(iTarget, iTargetLength));
	}

bool ZTextCollatorRep_NativeUTF32::Contains(const UTF16* iPattern, size_t iPatternLength,
	const UTF16* iTarget, size_t iTargetLength)
	{
	return this->Contains(Unicode::sAsUTF32(iPattern, iPatternLength),
		Unicode::sAsUTF32(iTarget, iTargetLength));
	}

// =================================================================================================

} // namespace ZooLib
