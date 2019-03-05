#ifndef __ZooLib_GameEngine_Tween_Rat_h__
#define __ZooLib_GameEngine_Tween_Rat_h__ 1
#include "zconfig.h"

#include "zoolib/GameEngine/Tween.h"
#include "zoolib/GameEngine/Types.h"

namespace ZooLib {
namespace GameEngine {

typedef Tween<Rat> Tween_Rat;

// =================================================================================================
// MARK: - RatRegistration

class RatRegistration
	{
public:
	typedef ZRef<Tween_Rat>(*Fun)(const Map& iMap);

	RatRegistration(const string8& iCtorName, Fun iFun);

	static ZRef<Tween_Rat> sCtor(const string8& iCtorName, const Map& iMap);
	};

} // namespace GameEngine
} // namespace ZooLib

#endif // __GameEngine_Tween_Rat_h__
