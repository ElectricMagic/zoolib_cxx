// Copyright (c) 2007 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/Apple/Util_NS_ZZ.h"

#if ZCONFIG_SPI_Enabled(CocoaFoundation)

#include "zoolib/Apple/Util_NS.h"

#include "zoolib/Unicode.h"
#include "zoolib/UTCDateTime.h"
#include "zoolib/Val_ZZ.h"

#import <Foundation/NSDate.h>
#import <Foundation/NSString.h>

#include <objc/runtime.h>

using std::vector;

using namespace ZooLib;

namespace ZooLib {
namespace Util_NS {

// =================================================================================================
#pragma mark - Util_NS

ZQ<Val_ZZ> sQAsZZ(NSObject* iVal)
	{
	if (iVal)
		{
		@try
			{
			return [iVal asZZ];
			}
		@catch (id ex)
			{}
		}
	return null;
	}

Val_ZZ sAsZZ(NSObject* iVal)
	{ return sQAsZZ(iVal) | Val_ZZ(); }

NSObject* sAsNSObject(const Val_ZZ& iVal)
	{
	if (false)
		{}
	else if (iVal.IsNull())
		{
		return [NSNull null];
		}
	else if (const string8* theValue = iVal.PGet<string8>())
		{
		return sString(*theValue);
		}
	else if (const vector<char>* theValue = iVal.PGet<vector<char>>())
		{
		if (size_t theSize = theValue->size())
			return sData(&(*theValue)[0], theSize);
		else
			return sData();
		}
	else if (const Data_ZZ* theValue = iVal.PGet<Data_ZZ>())
		{
		if (size_t theSize = theValue->GetSize())
			return sData(theValue->GetPtr(), theSize);
		else
			return sData();
		}
	else if (const Seq_ZZ* theValue = iVal.PGet<Seq_ZZ>())
		{
		NSMutableArray* theArray = sArrayMutable();
		for (size_t xx = 0, count = theValue->Size(); xx < count; ++xx)
			[theArray addObject:sAsNSObject(theValue->Get(xx))];
		return theArray;
		}
	else if (const Map_ZZ* theValue = iVal.PGet<Map_ZZ>())
		{
		NSMutableDictionary* theDictionary = sDictionaryMutable();
		for (Map_ZZ::Index_t ii = theValue->Begin(), end = theValue->End(); ii != end; ++ii)
			{
			[theDictionary
				setObject:sAsNSObject(theValue->Get(ii))
				forKey:sString(theValue->NameOf(ii))];
			}
		return theDictionary;
		}
	else if (const bool* theValue = iVal.PGet<bool>())
		{
		return [NSNumber numberWithBool:(BOOL)*theValue];
		}
	else if (const UTCDateTime* theValue = iVal.PGet<UTCDateTime>())
		{
		return [NSDate dateWithTimeIntervalSince1970:sGet(*theValue)];
		}
	else if (const char* theValue = iVal.PGet<char>())
		{
		return [NSNumber numberWithChar:*theValue];
		}
	else if (const unsigned char* theValue = iVal.PGet<unsigned char>())
		{
		return [NSNumber numberWithUnsignedChar:*theValue];
		}
	else if (const signed char* theValue = iVal.PGet<signed char>())
		{
		return [NSNumber numberWithChar:*theValue];
		}
	else if (const short* theValue = iVal.PGet<short>())
		{
		return [NSNumber numberWithShort:*theValue];
		}
	else if (const unsigned short* theValue = iVal.PGet<unsigned short>())
		{
		return [NSNumber numberWithUnsignedShort:*theValue];
		}
	else if (const int* theValue = iVal.PGet<int>())
		{
		return [NSNumber numberWithInt:*theValue];
		}
	else if (const unsigned int* theValue = iVal.PGet<unsigned int>())
		{
		return [NSNumber numberWithUnsignedInt:*theValue];
		}
	else if (const long* theValue = iVal.PGet<long>())
		{
		return [NSNumber numberWithLong:*theValue];
		}
	else if (const unsigned long* theValue = iVal.PGet<unsigned long>())
		{
		return [NSNumber numberWithUnsignedLong:*theValue];
		}
	else if (const int64* theValue = iVal.PGet<int64>())
		{
		return [NSNumber numberWithLongLong:(long long)*theValue];
		}
	else if (const uint64* theValue = iVal.PGet<uint64>())
		{
		return [NSNumber numberWithUnsignedLongLong:(unsigned long long)*theValue];
		}
	else if (const float* theValue = iVal.PGet<float>())
		{
		return [NSNumber numberWithFloat:*theValue];
		}
	else if (const double* theValue = iVal.PGet<double>())
		{
		return [NSNumber numberWithDouble:*theValue];
		}
	else if (const long double* theValue = iVal.PGet<long double>())
		{
		return [NSNumber numberWithDouble:double(*theValue)];
		}

	return nullptr;
	}

} // namespace Util_NS
} // namespace ZooLib

// =================================================================================================
@implementation NSObject (ZooLib_asZZ)

-(Val_ZZ)asZZ
	{
	@throw [NSException
		exceptionWithName:@"unhandled"
		reason:@"Unhandled type"
		userInfo:nil];
	}

@end

// =================================================================================================
@interface NSNull (ZooLib_asZZ)
-(Val_ZZ)asZZ;
@end

@implementation NSNull (ZooLib_asZZ)

-(Val_ZZ)asZZ
	{ return Val_ZZ(); }

@end

// =================================================================================================
@interface NSDictionary (ZooLib_asZZ)
-(Val_ZZ)asZZ;
@end

@implementation NSDictionary (ZooLib_asZZ)

-(Val_ZZ)asZZ
	{
	Map_ZZ result;
	for (id theKey, ii = [self keyEnumerator]; (theKey = [ii nextObject]); /*no inc*/)
		{
		const string8 theName = Util_NS::sAsUTF8((NSString*)theKey);
		const Val_ZZ theVal = [[self objectForKey:theKey] asZZ];
		result.Set(theName, theVal);
		}
	return result;
	}

@end

// =================================================================================================
@interface NSArray (ZooLib_asZZ)
-(Val_ZZ)asZZ;
@end

@implementation NSArray (ZooLib_asZZ)

-(Val_ZZ)asZZ
	{
	Seq_ZZ result;
	for (id theValue, ii = [self objectEnumerator]; (theValue = [ii nextObject]); /*no inc*/)
		result.Append([theValue asZZ].As<Val_ZZ>());
	return result;
	}

@end

// =================================================================================================
@interface NSData (ZooLib_asZZ)
-(Val_ZZ)asZZ;
@end

@implementation NSData (ZooLib_asZZ)

-(Val_ZZ)asZZ
	{ return Data_ZZ([self bytes], [self length]); }

@end

// =================================================================================================
@interface NSString (ZooLib_asZZ)
-(Val_ZZ)asZZ;
@end

@implementation NSString (ZooLib_asZZ)

-(Val_ZZ)asZZ
	{ return string8([self UTF8String]); }

@end

// =================================================================================================
@interface NSNumber (ZooLib_asZZ)
-(Val_ZZ)asZZ;
@end

@implementation NSNumber (ZooLib_asZZ)

-(Val_ZZ)asZZ
	{
	// c.f. PullPush_NS

	if (const char* type = [self objCType])
		{
		switch (type[0])
			{
			// _C_xxx constants are defined in objc/runtime.h
			case _C_CHR:
				{
				NSString* selfClassName = NSStringFromClass([self class]);
				// CoreFoundation
				if ([@"__NSCFBoolean" isEqualToString:selfClassName])
					return [self boolValue];
				// Kotlin
				if ([@"SharedBoolean" isEqualToString:selfClassName])
					return [self boolValue];
				return [self charValue];
				}
			case _C_UCHR: return [self unsignedCharValue];
			case _C_SHT: return [self shortValue];
			case _C_USHT: return [self unsignedShortValue];
			case _C_INT: return [self intValue];
			case _C_UINT: return [self unsignedIntValue];
			case _C_LNG: return [self longValue];
			case _C_ULNG: return [self unsignedLongValue];
			case _C_LNG_LNG: return [self longLongValue];
			case _C_ULNG_LNG: return [self unsignedLongLongValue];
			case _C_FLT: return [self floatValue];
			case _C_DBL: return [self doubleValue];
			case _C_BOOL: return [self boolValue];
			}
		}

	@throw [NSException
		exceptionWithName:@"unhandled"
		reason:@"Unhandled NSNumber variant"
		userInfo:nil];
	}

@end

// =================================================================================================
@interface NSDate (ZooLib_asZZ)
-(Val_ZZ)asZZ;
@end

@implementation NSDate (ZooLib_asZZ)

-(Val_ZZ)asZZ
	{ return ZooLib::UTCDateTime([self timeIntervalSince1970]); }

@end

#endif // ZCONFIG_SPI_Enabled(CocoaFoundation)


//@"c" : @"char",
//@"i" : @"int",
//@"s" : @"short",
//@"l" : @"long",
//@"q" : @"long long",
//@"C" : @"unsigned char",
//@"I" : @"unsigned int",
//@"S" : @"unsigned short",
//@"L" : @"unsigned long",
//@"Q" : @"unsigned long long",
//@"f" : @"float",
//@"d" : @"double",
//@"B" : @"bool",

//@"v" : @"void",
//@"*" : @"char *",
//@"r" : @"const char",     /* Used with char * return types */
//@"@" : @"id",
//@"#" : @"Class",
//@":" : @"SEL",
//@"?" : @"*",
//@"{" : @"struct"
