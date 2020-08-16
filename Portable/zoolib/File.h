// Copyright (c) 2000 Andrew Green and Learning in Motion, Inc.
// MIT License. http://www.zoolib.org

#ifndef __ZooLib_File_h__
#define __ZooLib_File_h__ 1
#include "zconfig.h"

#include "zoolib/Compat_NonCopyable.h"
#include "zoolib/Channer_Bin.h"
#include "zoolib/FunctionChain.h"
#include "zoolib/Time.h"
#include "zoolib/Trail.h"
#include "zoolib/Util_Relops.h"

#include "zoolib/ZQ.h"
#include "zoolib/ZThread.h"

namespace ZooLib {

class FileIterRep;
class FileLoc;
class FileSpec;

// =================================================================================================
#pragma mark - File

namespace File {

enum Error
	{
	errorNone,
	errorGeneric,
	errorDoesntExist,
	errorAlreadyExists,
	errorInvalidFileSpec,
	errorIllegalFileName,
	errorNoPermission,
	errorWrongTypeForOperation,
	errorReadPastEOF,
	errorInsufficientSpace
	};

enum Kind
	{
	kindNone,
	kindFile,
	kindLink,
	kindDir
	};

} // namespace File

// =================================================================================================
#pragma mark - FileSpec

/// Represents a node in the file system.

class FileSpec
	{
public:
	typedef File::Error Error;

	FileSpec();
	FileSpec(const FileSpec& iSpec);
	FileSpec(FileLoc* iLoc);
	FileSpec(const ZP<FileLoc>& iLoc);
	FileSpec(const ZP<FileLoc>& iLoc, const std::string& iComp);

	template <class I>
	FileSpec(const ZP<FileLoc>& iLoc, const I& iBegin, const I& iEnd)
	:	fLoc(iLoc),
		fComps(iBegin, iEnd)
		{}

	template <class I>
	FileSpec(const ZP<FileLoc>& iLoc,
		const I& iBegin, const I& iEnd, const std::string& iAdditional)
	:	fLoc(iLoc),
		fComps(iBegin, iEnd)
		{ fComps.push_back(iAdditional); }

	FileSpec(const std::string& iPath);

	ZMACRO_operator_bool(FileSpec, operator_bool) const;

	static FileSpec sCWD();
	static FileSpec sRoot();
	static FileSpec sApp();

	std::string Name() const;
	FileSpec Parent() const;
	FileSpec Child(const std::string& iName) const;
	FileSpec Sibling(const std::string& iName) const;
	FileSpec Follow(const Trail& iTrail) const;
	FileSpec Ancestor(size_t iCount) const;
	FileSpec Descendant(const std::string* iComps, size_t iCount) const;

	FileSpec Follow() const;

	std::string AsString() const;
	std::string AsString_Native() const;

	ZQ<Trail> TrailTo(const FileSpec& oDest) const;
	ZQ<Trail> TrailFrom(const FileSpec& iSource) const;

	File::Kind Kind() const;

	bool IsRoot() const;
	bool IsFile() const;
	bool IsDir() const;
	bool IsSymLink() const;
	bool Exists() const;

	uint64 Size() const;
	double TimeCreated() const;
	double TimeModified() const;

	bool SetCreatorAndType(uint32 iCreator, uint32 iType) const;

	FileSpec CreateDir() const;

	FileSpec MoveTo(const FileSpec& oDest) const;
	bool Delete() const;

	// Open/create with stream API (stateful).
	ZP<ChannerR_Bin> OpenR(bool iPreventWriters = false) const;
	ZP<ChannerRPos_Bin> OpenRPos(bool iPreventWriters = false) const;
	ZP<ChannerW_Bin> OpenW(bool iPreventWriters = true) const;
	ZP<ChannerWPos_Bin> OpenWPos(bool iPreventWriters = true) const;
	ZP<ChannerRWPos_Bin> OpenRWPos(bool iPreventWriters = true) const;

	ZP<ChannerWPos_Bin> CreateWPos(bool iOpenExisting, bool iPreventWriters = true) const;
	ZP<ChannerRWPos_Bin> CreateRWPos(bool iOpenExisting, bool iPreventWriters = true) const;

	// As ever, do not abuse...
	ZP<FileLoc> GetFileLoc() const;

private:
	// It's not generally possible to relate FileSpecs to one another. Disable it.
	void operator<(const FileSpec& iOther);

	ZP<FileLoc> pPhysicalLoc() const;

	ZP<FileLoc> fLoc;
	std::vector<std::string> fComps;
	};

// Disable other relops.
template <>
struct RelopsTraits_HasLT<FileSpec> : public RelopsTraits_Has {};

// =================================================================================================
#pragma mark - FileIter

/// An iterator that provides access to the children of a directory.

class FileIter
	{
public:
	FileIter();
	FileIter(const FileIter& iOther);
	FileIter(const FileSpec& iRoot);
	~FileIter();
	FileIter& operator=(const FileIter& iOther);

	ZMACRO_operator_bool(FileIter, operator_bool) const;
	FileIter& Advance();
	FileSpec Current() const;
	std::string CurrentName() const;

private:
	ZP<FileIterRep> fRep;
	};

// =================================================================================================
#pragma mark - FileTreeIter

/// An iterator that provides access to every node descending from a FileSpec.

class FileTreeIter
	{
public:
	FileTreeIter();
	FileTreeIter(const FileTreeIter& iOther);
	FileTreeIter(const FileSpec& iRoot);
	~FileTreeIter();
	FileTreeIter& operator=(const FileTreeIter& iOther);

	ZMACRO_operator_bool(FileTreeIter, operator_bool) const;
	FileTreeIter& Advance();
	FileSpec Current() const;
	std::string CurrentName() const;

private:
	FileSpec fCurrent;
	std::vector<FileIter> fStack;
	};

// =================================================================================================
#pragma mark - FileLoc

class FileLoc : public Counted
	{
protected:
	FileLoc();

public:
	enum ELoc { eLoc_Root, eLoc_CWD, eLoc_App };

	virtual ~FileLoc();

	virtual ZP<FileIterRep> CreateIterRep();

	virtual std::string GetName() const = 0;
	virtual ZQ<Trail> TrailTo(ZP<FileLoc> oDest) const = 0;

	virtual ZP<FileLoc> GetAncestor(size_t iCount);
	virtual ZP<FileLoc> GetParent() = 0;
	virtual ZP<FileLoc> GetDescendant(
		const std::string* iComps, size_t iCount) = 0;
	virtual bool IsRoot() = 0;

	virtual ZP<FileLoc> Follow();

	virtual std::string AsString_POSIX(const std::string* iComps, size_t iCount) = 0;
	virtual std::string AsString_Native(const std::string* iComps, size_t iCount) = 0;

	virtual File::Kind Kind() = 0;
	virtual uint64 Size() = 0;
	virtual double TimeCreated() = 0;
	virtual double TimeModified() = 0;

	virtual bool SetCreatorAndType(uint32 iCreator, uint32 iType);

	virtual ZP<FileLoc> CreateDir() = 0;

	virtual ZP<FileLoc> MoveTo(ZP<FileLoc> oDest) = 0;
	virtual bool Delete() = 0;

	virtual ZP<ChannerR_Bin> OpenR(bool iPreventWriters);
	virtual ZP<ChannerRPos_Bin> OpenRPos(bool iPreventWriters);
	virtual ZP<ChannerW_Bin> OpenW(bool iPreventWriters);
	virtual ZP<ChannerWPos_Bin> OpenWPos(bool iPreventWriters);
	virtual ZP<ChannerRWPos_Bin> OpenRWPos(bool iPreventWriters);

	virtual ZP<ChannerWPos_Bin> CreateWPos(bool iOpenExisting, bool iPreventWriters);
	virtual ZP<ChannerRWPos_Bin> CreateRWPos(bool iOpenExisting, bool iPreventWriters);
	};

// =================================================================================================
#pragma mark - FileIterRep

class FileIterRep : public Counted
	{
protected:
	FileIterRep();

public:
	virtual ~FileIterRep();

	virtual bool HasValue() = 0;
	virtual void Advance() = 0;
	virtual FileSpec Current() = 0;
	virtual std::string CurrentName() const = 0;

	virtual ZP<FileIterRep> Clone() = 0;
	};

// =================================================================================================
#pragma mark - FileIterRep_Std

class FileIterRep_Std : public FileIterRep
	{
public:
	class RealRep;

	FileIterRep_Std(ZP<RealRep> iRealRep, size_t iIndex);
	virtual ~FileIterRep_Std();

// From FileIterRep
	virtual bool HasValue();
	virtual void Advance();
	virtual FileSpec Current();
	virtual std::string CurrentName() const;

	virtual ZP<FileIterRep> Clone();

private:
	ZP<RealRep> fRealRep;
	size_t fIndex;
	};

// =================================================================================================
#pragma mark - FileIterRep_Std::RealRep

class FileIterRep_Std::RealRep : public Counted
	{
protected:
	RealRep() {};

public:
	virtual bool HasValue(size_t iIndex) = 0;
	virtual FileSpec GetSpec(size_t iIndex) = 0;
	virtual std::string GetName(size_t iIndex) = 0;
	};

} // namespace ZooLib

#endif // __ZooLib_File_h__
