// Copyright (c) 2019 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_GameEngine_AssetCatalog_h__
#define __ZooLib_GameEngine_AssetCatalog_h__ 1
#include "zconfig.h"

#include "zoolib/Callable.h"
#include "zoolib/Compat_unordered_map.h"

#include "zoolib/GameEngine/Rendered.h"

#include <map>
#include <set>

namespace ZooLib {
namespace GameEngine {

// =================================================================================================
#pragma mark - Texture_BoundsQ_Mat

struct Texture_BoundsQ_Mat
	{
	ZP<Texture> fTexture;
	ZQ<GRect> fBoundsQ;
	GRect fInset;
	Mat fMat;
	};

// =================================================================================================
#pragma mark - AssetCatalog

class AssetCatalog
:	public Counted
	{
public:
	AssetCatalog();
	virtual ~AssetCatalog();

	typedef Callable<ZP<Texture>()> Callable_TextureMaker;

	void Load(const NameFrame& iNameFrame, int iPriority);

	size_t FrameCount(const Name& iName);
	void Get(const NameFrame& iNameFrame, std::vector<Texture_BoundsQ_Mat>& ioResult);

	void ExternalPurgeHasOccurred();

	void Purge();

	void Kill();

	void InstallSheet(const Name& iName, const ZP<Callable_TextureMaker>& iTM);
	void Set_Processed(const Map_ZZ& iMap);

	class SheetCatalog;

private:
	bool pGet(const Name& iName, size_t iFrame, int iPriority,
		std::vector<Texture_BoundsQ_Mat>* ioResult);

	ZP<SheetCatalog> fSheetCatalog;

	Map_ZZ fMap;

	struct Sheet_BoundsQ_Mat
		{
		Name fSheetName;
		ZQ<GRect> fBoundsQ;
		GRect fInset;
		Mat fMat;		
		};

	struct Frame
		{
		ZQ<void> fLoaded;
		std::vector<Sheet_BoundsQ_Mat> fSheet_BoundsQ_Mats;
		};
	
	struct Entry
		{
		Name fDefaultSheetName;
		ZQ<GRect> fDefaultBoundsQ;
		CVec3 fBaseScale;
		CVec3 fBaseReg;
		GRect fBaseInset;
		
		std::vector<Frame> fFramesVec;
		Seq_ZZ fFramesSeq;
		};

	#if 0 && ZCONFIG_SPI_Enabled(unordered_map)
		unordered_map<Name,Entry> fMap_NameEntry;
	#else
		std::map<Name,Entry> fMap_NameEntry;
	#endif

	MtxF fMtx;
	};

// =================================================================================================
#pragma mark - Globals

size_t sToonFrameCount(const Name& iName);

} // namespace GameEngine
} // namespace ZooLib

#endif // __ZooLib_GameEngine_AssetCatalog_h__
