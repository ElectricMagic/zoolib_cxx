#include "zoolib/GameEngine/Cog.h"

namespace ZooLib {
namespace GameEngine {

Cog sCog_Button(
	const ZRef<TouchListener>& iTouchListener,
	const Cog& iCog_UpOut,
	const Cog& iCog_DownIn,
	const Cog& iCog_DownOut,
	const Cog& iCog_Pushed
	);

} // namespace GameEngine
} // namespace ZooLib
