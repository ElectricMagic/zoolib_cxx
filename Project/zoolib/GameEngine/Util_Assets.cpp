// Copyright (c) 2019 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/GameEngine/Util_Assets.h"
#include "zoolib/GameEngine/Util.h" // For sQReadMap_ZZ

#include "zoolib/Util_string.h"

namespace ZooLib {
namespace GameEngine {

using std::map;
using namespace Util_string;

// =================================================================================================
#pragma mark -

bool sReadAnim(const FileSpec& iFS, map<string8,FileSpec>& oFiles, Map_ZZ& oMap)
	{
	if (not iFS.IsDir())
		return false;

	for (FileIter iter = iFS; iter; iter.Advance())
		{
		const FileSpec current = iter.Current();
		if (ZQ<string8> woSuffix = sQWithoutSuffix(current.Name(), ".png"))
			oFiles[current.Name()] = current;
		}

	if (oFiles.empty())
		return false;

	oMap = sGet(sQReadMap_ZZ(iFS.Child("meta.txt")));

	return true;
	}

} // namespace GameEngine
} // namespace ZooLib
