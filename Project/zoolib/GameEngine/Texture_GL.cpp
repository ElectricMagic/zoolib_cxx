#include "zoolib/GameEngine/Texture_GL.h"

#include "zoolib/Callable_Function.h"

#include "zoolib/GameEngine/Util.h"
#include "zoolib/OpenGL/Util.h"

namespace ZooLib {
namespace GameEngine {

using namespace OpenGL;

using ZDCPixmapNS::PixelDescRep_Color;
using ZDCPixmapNS::PixelDescRep_Gray;

// =================================================================================================
#pragma mark - Helpers

static int spNextPowerOfTwo(int input)
	{	
	int result = 1;
	while (result < input)
		result <<= 1;
	return result;
	}

// =================================================================================================
#pragma mark - Texture_GL

Texture_GL::Texture_GL(ZPointPOD iDrawnSize)
:	fDrawnSize(iDrawnSize)
,	fTextureID(0)
,	fIsAlphaOnly(false)
	{}

Texture_GL::Texture_GL(ZPointPOD iTextureSize, TextureID iTextureID, bool iIsAlphaOnly)
:	fTextureSize(iTextureSize)
,	fDrawnSize(iTextureSize)
,	fTextureID(iTextureID)
,	fIsAlphaOnly(iIsAlphaOnly)
	{}

Texture_GL::Texture_GL(const ZDCPixmap& iPixmap)
:	fPixmap(iPixmap)
,	fDrawnSize(fPixmap.Size())
,	fIsAlphaOnly(false)
	{
	using namespace ZDCPixmapNS;

	// For the momemnt, insist on RGBA (LE)
	if (ZRef<PixelDescRep_Color> thePDRep = fPixmap.GetPixelDesc().GetRep().DynamicCast<PixelDescRep_Color>())
		{
		ZooLib::uint32 theR, theG, theB, theA;
		thePDRep->GetMasks(theR, theG, theB, theA);

		const PixvalDesc thePVDesc = fPixmap.GetRaster()->GetRasterDesc().fPixvalDesc;
		if (thePVDesc.fBigEndian)
			{
			ZAssert(theR == 0xFF000000);
			ZAssert(theG == 0x00FF0000);
			ZAssert(theB == 0x0000FF00);
			ZAssert(theA == 0x000000FF);
			}
		else
			{
			ZAssert(theA == 0xFF000000);
			ZAssert(theB == 0x00FF0000);
			ZAssert(theG == 0x0000FF00);
			ZAssert(theR == 0x000000FF);
			}
		}
	else if (ZRef<PixelDescRep_Gray> thePDRep = fPixmap.GetPixelDesc().GetRep().DynamicCast<PixelDescRep_Gray>())
		{
		uint32 maskL, maskA;
		thePDRep->GetMasks(maskL, maskA);
		ZAssert(maskL == 0);
		ZAssert(maskA == 0xFF);
		fIsAlphaOnly = true;
		}
	else
		{
		ZUnimplemented();
		}
	// We should also require that pixmap bounds encompass the entirety of the raster.
	}

Texture_GL::~Texture_GL()
	{
	if (fTextureID)
		::glDeleteTextures(1, &fTextureID.Get());
	}

ZPointPOD Texture_GL::GetDrawnSize()
	{ return fDrawnSize; }

void Texture_GL::Get(TextureID& oTextureID, ZPointPOD& oTextureSize)
	{
	if (not fTextureID)
		this->pMakeTexture();
	oTextureID = fTextureID;
	oTextureSize = fTextureSize;
	}

TextureID Texture_GL::GetTextureID()
	{
	if (not fTextureID)
		this->pMakeTexture();
	return fTextureID;
	}

ZPointPOD Texture_GL::GetTextureSize()
	{
	if (not fTextureID)
		this->pMakeTexture();
	return fTextureSize;
	}

bool Texture_GL::GetIsAlphaOnly()
	{ return fIsAlphaOnly; }

void Texture_GL::pMakeTexture()
	{
	ZAssert(not fTextureID);

	SaveSetRestore_ActiveTexture ssr_ActiveTexture(GL_TEXTURE0);

	GLuint theTextureID;
	::glGenTextures(1, &theTextureID);

	SaveSetRestore_BindTexture_2D ssr_BindTexture_2D(theTextureID);

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	#if defined(GL_VERSION_2_1)
		// GL_CLAMP_TO_BORDER_EXT ??
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	#else
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	#endif

	static bool spNeedPOT = false; // Need textures that are a `Power Of Two` in size?

	const void* baseAddress = nullptr;
	GLenum format = GL_RGBA;

	if (fPixmap)
		{
		baseAddress = fPixmap.GetBaseAddress();
		if (fPixmap.GetPixelDesc().GetRep().DynamicCast<PixelDescRep_Gray>())
			format = GL_ALPHA;
		}

	if (not spNeedPOT)
		{
		fTextureSize = fDrawnSize;

		::glTexImage2D(GL_TEXTURE_2D, 0, format,
			X(fTextureSize), Y(fTextureSize), 0,
			format, GL_UNSIGNED_BYTE, baseAddress);
		}

	if (spNeedPOT || ::glGetError())
		{
		fTextureSize =
			sPointPOD(spNextPowerOfTwo(X(fDrawnSize)), spNextPowerOfTwo(Y(fDrawnSize)));

		::glTexImage2D(GL_TEXTURE_2D, 0, format,
			X(fTextureSize), Y(fTextureSize), 0,
			format, GL_UNSIGNED_BYTE, nullptr);

		if (baseAddress)
			{
			::glTexSubImage2D(GL_TEXTURE_2D, 0,
				0, 0,
				X(fDrawnSize), Y(fDrawnSize),
				format, GL_UNSIGNED_BYTE, baseAddress);
			}

		if (not ::glGetError())
			spNeedPOT = true;
		}

	fPixelCount = X(fTextureSize) * Y(fTextureSize);
	fTextureID = theTextureID;
	fPixmap = null;
	}

static ZRef<Texture> spTextureFromPixmap(const ZDCPixmap& iPixmap)
	{
	if (iPixmap)
		return new Texture_GL(iPixmap);
	return null;
	}

ZRef<Callable_TextureFromPixmap> sCallable_TextureFromPixmap_GL()
	{
	GEMACRO_Callable(spCallable, spTextureFromPixmap);
	return spCallable;
	}

} // namespace GameEngine
} // namespace ZooLib
