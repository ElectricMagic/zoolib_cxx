// Copyright (c) 2002 Andrew Green and Learning in Motion, Inc.
// MIT License. http://www.zoolib.org

#ifndef __ZooLib_POSIX_File_POSIX_h__
#define __ZooLib_POSIX_File_POSIX_h__ 1
#include "zconfig.h"
#include "zoolib/ZCONFIG_API.h"
#include "zoolib/ZCONFIG_SPI.h"

#include "zoolib/ChanR_Bin.h"
#include "zoolib/ChanW_Bin.h"
#include "zoolib/File.h"

#ifndef ZCONFIG_API_Avail__File_POSIX
	#define ZCONFIG_API_Avail__File_POSIX ZCONFIG_SPI_Enabled(POSIX)
#endif

#ifndef ZCONFIG_API_Desired__File_POSIX
	#define ZCONFIG_API_Desired__File_POSIX 1
#endif

#if ZCONFIG_API_Enabled(File_POSIX)

ZMACRO_MSVCStaticLib_Reference(File_POSIX)

namespace ZooLib {

// =================================================================================================
#pragma mark - FileLoc_POSIX

class FileLoc_POSIX : public FileLoc
	{
public:
	static ZP<FileLoc_POSIX> sGet_CWD();
	static ZP<FileLoc_POSIX> sGet_Root();
	static ZP<FileLoc_POSIX> sGet_App();

	FileLoc_POSIX(bool iIsAtRoot);
	FileLoc_POSIX(bool iIsAtRoot, const std::vector<std::string>& iComps);
	FileLoc_POSIX(bool iIsAtRoot, const std::string* iComps, size_t iCount);
	FileLoc_POSIX(bool iIsAtRoot, std::vector<std::string>* ioComps, const IKnowWhatIAmDoing_t&);
	virtual ~FileLoc_POSIX();

// From FileLoc
	virtual ZP<FileIterRep> CreateIterRep();

	virtual std::string GetName() const;
	virtual ZQ<Trail> TrailTo(ZP<FileLoc> oDest) const;

	virtual ZP<FileLoc> GetParent();
	virtual ZP<FileLoc> GetDescendant(
		const std::string* iComps, size_t iCount);

	virtual bool IsRoot();

	virtual ZP<FileLoc> Follow();

	virtual std::string AsString_POSIX(const std::string* iComps, size_t iCount);
	virtual std::string AsString_Native(const std::string* iComps, size_t iCount);

	virtual File::Kind Kind();
	virtual uint64 Size();
	virtual double TimeCreated();
	virtual double TimeModified();

	virtual ZP<FileLoc> CreateDir();

	virtual ZP<FileLoc> MoveTo(ZP<FileLoc> oDest);
	virtual bool Delete();

	virtual ZP<ChannerRPos_Bin> OpenRPos(bool iPreventWriters);
	virtual ZP<ChannerWPos_Bin> OpenWPos(bool iPreventWriters);
	virtual ZP<ChannerRWPos_Bin> OpenRWPos(bool iPreventWriters);

	virtual ZP<ChannerWPos_Bin> CreateWPos(bool iOpenExisting, bool iPreventWriters);
	virtual ZP<ChannerRWPos_Bin> CreateRWPos(bool iOpenExisting, bool iPreventWriters);

	std::string pGetPath();

private:
	bool fIsAtRoot;
	std::vector<std::string> fComps;
	};

} // namespace ZooLib

#endif // ZCONFIG_API_Enabled(File_POSIX)

#endif // __ZooLib_POSIX_File_POSIX_h__
