/* -------------------------------------------------------------------------------------------------
Copyright (c) 2002 Andrew Green
http://www.zoolib.org

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------------------------- */

#include "zoolib/ZNetscape_GuestFactory_Mac.h"

#include "zoolib/ZCONFIG_SPI.h"

#if ZCONFIG_SPI_Enabled(CoreFoundation)

#include "zoolib/ZDebug.h"
#include "zoolib/ZLog.h"
#include "zoolib/ZMemory.h"
#include "zoolib/ZUtil_MacOSX.h"

#include ZMACINCLUDE(CoreFoundation,CFBundle.h)

#include<vector>

using std::vector;

#if __MACH__
#	include <mach-o/dyld.h> // For NSModule
#endif

// =================================================================================================
#pragma mark -
#pragma mark * Helper functions

#if __MACH__

static void* sLookup(NSModule iModule, const char* iName)
	{
	if (NSSymbol theSymbol = ::NSLookupSymbolInModule(iModule, iName))
		return ::NSAddressOfSymbol(theSymbol);
	return nil;
	}

static NSModule sLoadNSModule(CFBundleRef iBundleRef)
	{
	NSModule module = nil;

	if (CFURLRef executableURL = ::CFBundleCopyExecutableURL(iBundleRef))
		{
		char buff[PATH_MAX];

		if (::CFURLGetFileSystemRepresentation(executableURL, true, (UInt8*)buff, PATH_MAX))
			{
			NSObjectFileImage image;
			if (NSObjectFileImageSuccess == ::NSCreateObjectFileImageFromFile(buff, &image))
				{
				module = ::NSLinkModule(
					image, buff,
					NSLINKMODULE_OPTION_BINDNOW
					| NSLINKMODULE_OPTION_PRIVATE
					| NSLINKMODULE_OPTION_RETURN_ON_ERROR);
				}
			}
		::CFRelease(executableURL);
		}

	return module;
	}

template <typename P>
P sLookup_T(NSModule iNSModule, const char* iName)
	{ return reinterpret_cast<P>(sLookup(iNSModule, iName)); }

#endif // __MACH__

static void* sLookup(CFBundleRef iBundleRef, CFStringRef iName)
	{ return ::CFBundleGetFunctionPointerForName(iBundleRef, iName); }

template <typename P>
P sLookup_T(CFBundleRef iBundleRef, CFStringRef iName)
	{ return reinterpret_cast<P>(sLookup(iBundleRef, iName)); }

// =================================================================================================
#pragma mark -
#pragma mark * GuestFactory_HostMachO

// We may well be wanting to load the plugin into an environment where it's already been loaded.
// GuestFactory_MacPlugin uses the Bundle mechanism to locate the plugin, but uses NSLinkModule
// to load and instantiate an independent copy of the library. In this way the library's static
// references to NPNetscapeFuncs is independent of any other instantiation of the library.

#if __MACH__

class GuestFactory_HostMachO : public ZNetscape::GuestFactory
	{
public:
	GuestFactory_HostMachO(CFPlugInRef iPlugInRef);
	virtual ~GuestFactory_HostMachO();

	virtual void GetEntryPoints(NPPluginFuncs& oNPPluginFuncs);

private:
	CFPlugInRef fPlugInRef;
	NSModule fNSModule;
	NPPluginFuncs fNPPluginFuncs;
	NPP_ShutdownProcPtr fShutdown;

	#if ZCONFIG(Processor,PPC)
		std::vector<char> fGlue_NPNF;
		std::vector<char> fGlue_PluginFuncs;
		std::vector<char> fGlue_Shutdown;
	#endif
	};

GuestFactory_HostMachO::GuestFactory_HostMachO(CFPlugInRef iPlugInRef)
:	fPlugInRef(iPlugInRef),
	fNSModule(nil)
	{
	::CFRetain(fPlugInRef);

	// Get local copies of our host's function pointers
	NPNetscapeFuncs localNPNF;
	GuestFactory::GetNPNF(localNPNF);

	// And clean out our plugin functions struct
	ZBlockZero(&fNPPluginFuncs, sizeof(fNPPluginFuncs));
	fNPPluginFuncs.size = sizeof(NPPluginFuncs);


	CFBundleRef theBundleRef = ::CFPlugInGetBundle(fPlugInRef);

	// Force the bundle to be physically loaded by asking for an entry point. If we
	// don't do this, and unload the bundle, then any subsequent loader will
	// get nils for any entry point. In Safari this manifests with a
	// "Internal error unloading bundle" log message.
	bool isMachOPlugin = sLookup(theBundleRef, CFSTR("NP_Initialize"));//??

	// This also tells us that the NP_Initialize entry point exists, so it's
	// a macho binary (CFM would only provide 'main'), and we should use NSModule
	// to load our independent instantiation.

	if (isMachOPlugin)
		{
		fNSModule = sLoadNSModule(theBundleRef);

		sLookup_T<NP_InitializeFuncPtr>
			(fNSModule, "_NP_Initialize")
			(&localNPNF);

		sLookup_T<NP_GetEntryPointsFuncPtr>
			(fNSModule, "_NP_GetEntryPoints")
			(&fNPPluginFuncs);

		fShutdown = sLookup_T<NPP_ShutdownProcPtr>(fNSModule, "_NP_Shutdown");
		}
	else
		{
		// There's no NP_Initialize, assume we're dealing with a plugin
		// that's expecting CFM-type behavior. On PPC that means that function
		// pointers we pass to the plugin must be callable by CFM code, although
		// this is MachO, and so we must pass pointers to CFM thunks in localNPNF.

		#if ZCONFIG(Processor,PPC)
			// Rework localNPNF as CFM-callable thunks
			ZUtil_MacOSX::sCreateThunks_MachOCalledByCFM(
				&localNPNF.geturl,
				(localNPNF.size - offsetof(NPNetscapeFuncs, geturl)) / sizeof(void*),
				fGlue_NPNF);
		#endif

		// Call main, which itself may be a CFM function, but the bundle function
		// lookup mechanism will have created a MachO-callable thunk for it.
		
		sLookup_T<MainFuncPtr>
			(theBundleRef, CFSTR("main"))
			(&localNPNF, &fNPPluginFuncs, &fShutdown);		


		#if ZCONFIG(Processor,PPC)
			// Rework fNPPluginFuncs and fShutDown as MachO-Callable thunks.
			ZUtil_MacOSX::sCreateThunks_CFMCalledByMachO(
				&fNPPluginFuncs.newp,
				(fNPPluginFuncs.size - offsetof(NPPluginFuncs, newp)) / sizeof(void*),
				fGlue_PluginFuncs);
			ZUtil_MacOSX::sCreateThunks_CFMCalledByMachO(&fShutdown, 1, fGlue_Shutdown);
		#endif		
		}
	}

GuestFactory_HostMachO::~GuestFactory_HostMachO()
	{
	fShutdown();

	if (fNSModule)
		{
		// We manually loaded our own NSModule, which we need to unload.
		::NSUnLinkModule(fNSModule, NSUNLINKMODULE_OPTION_NONE);
		}

	::CFRelease(fPlugInRef);
	}

void GuestFactory_HostMachO::GetEntryPoints(NPPluginFuncs& oNPPluginFuncs)
	{
	oNPPluginFuncs = fNPPluginFuncs;
	}

#endif // __MACH__

// =================================================================================================
#pragma mark -
#pragma mark * GuestFactory_HostCFM

#if ZCONFIG(Processor,PPC) && !__MACH__

#ifndef NPP_MainEntryUPP
	extern "C" {
	typedef NPError (*NP_InitializeFuncPtr)(NPNetscapeFuncs*);
	typedef NPError (*NP_GetEntryPointsFuncPtr)(NPPluginFuncs*);
	typedef void (*NPP_ShutdownProcPtr)();
//	typedef void (*BP_CreatePluginMIMETypesPreferencesFuncPtr)(void);
	typedef NPError (*MainFuncPtr)(NPNetscapeFuncs*, NPPluginFuncs*, NPP_ShutdownProcPtr*);
	} // extern "C"
#endif

class GuestFactory_HostCFM : public ZNetscape::GuestFactory
	{
public:
	GuestFactory_HostCFM(CFPlugInRef iPlugInRef);
	virtual ~GuestFactory_HostCFM();

	virtual void GetEntryPoints(NPPluginFuncs& oNPPluginFuncs);

private:
	CFPlugInRef fPlugInRef;
	NPPluginFuncs fNPPluginFuncs;
	NPP_ShutdownProcPtr fShutdown;

	#if ZCONFIG(Processor,PPC)
		std::vector<char> fGlue_NPNF;
		std::vector<char> fGlue_PluginFuncs;
		std::vector<char> fGlue_Shutdown;
	#endif
	};

GuestFactory_HostCFM::GuestFactory_HostCFM(CFPlugInRef iPlugInRef)
:	fPlugInRef(iPlugInRef)
	{
	::CFRetain(fPlugInRef);
	// Retain it twice. If it contains ObjC data it'll kill the
	// host application when we release.
	::CFRetain(fPlugInRef);

	// Get local copies of our host's function pointers
	NPNetscapeFuncs localNPNF;
	GuestFactory::GetNPNF(localNPNF);

	// And clean out our plugin functions struct
	ZBlockZero(&fNPPluginFuncs, sizeof(fNPPluginFuncs));
	fNPPluginFuncs.size = sizeof(NPPluginFuncs);

	CFBundleRef theBundleRef = ::CFPlugInGetBundle(fPlugInRef);

	MainFuncPtr theMain = nil;
	#if ZCONFIG(Processor,PPC)
		// We're PowerPC -- look for main(), and if it's there we can just call it.
		theMain = sLookup_T<MainFuncPtr>(theBundleRef, CFSTR("main"));
	#endif
	if (theMain)
		{
		theMain(&localNPNF, &fNPPluginFuncs, &fShutdown);
		}
	else
		{
		#if ZCONFIG(Processor,PPC)
			// Rework localNPNF as MachO-callable thunks
			ZUtil_MacOSX::sCreateThunks_CFMCalledByMachO(
				&localNPNF.geturl,
				(localNPNF.size - offsetof(NPNetscapeFuncs, geturl)) / sizeof(void*),
				fGlue_NPNF);
		#endif

		fShutdown = sLookup_T<NPP_ShutdownProcPtr>(theBundleRef, CFSTR("NP_Shutdown"));

		NP_InitializeFuncPtr theInit = 
		sLookup_T<NP_InitializeFuncPtr>
			(theBundleRef, CFSTR("NP_Initialize"));
			theInit
			(&localNPNF);

		NP_GetEntryPointsFuncPtr theEntryPoints =
		sLookup_T<NP_GetEntryPointsFuncPtr>
			(theBundleRef, CFSTR("NP_GetEntryPoints"));
			theEntryPoints
			(&fNPPluginFuncs);


		#if ZCONFIG(Processor,PPC)
			// Rework fNPPluginFuncs and fShutDown as CFM-Callable thunks.
			ZUtil_MacOSX::sCreateThunks_MachOCalledByCFM(
				&fNPPluginFuncs.newp,
				(fNPPluginFuncs.size - offsetof(NPPluginFuncs, newp)) / sizeof(void*),
				fGlue_PluginFuncs);
			ZUtil_MacOSX::sCreateThunks_MachOCalledByCFM(&fShutdown, 1, fGlue_Shutdown);
		#endif		
		}
	}

GuestFactory_HostCFM::~GuestFactory_HostCFM()
	{
	int firstRefCount = ::CFGetRetainCount(fPlugInRef);

	fShutdown();

	int secondRefCount = ::CFGetRetainCount(fPlugInRef);

	::CFRelease(fPlugInRef);

	int thirdRefCount = ::CFGetRetainCount(fPlugInRef);
	}

void GuestFactory_HostCFM::GetEntryPoints(NPPluginFuncs& oNPPluginFuncs)
	{
	oNPPluginFuncs = fNPPluginFuncs;
	}

#endif // ZCONFIG(Processor,PPC) && !__MACH__

#endif // ZCONFIG_SPI_Enabled(CoreFoundation)

// =================================================================================================
#pragma mark -
#pragma mark * ZNetscape

#if ZCONFIG_SPI_Enabled(CoreFoundation)

static ZRef<ZNetscape::GuestFactory> sMakeGuestFactory_MacPlugin(CFPlugInRef iPlugInRef)
	{
	#if __MACH__
		return new GuestFactory_HostMachO(iPlugInRef);
	#elif ZCONFIG(Processor,PPC)
		return new GuestFactory_HostCFM(iPlugInRef);
	#endif

	return ZRef<ZNetscape::GuestFactory>();
	}

#endif // ZCONFIG_SPI_Enabled(CoreFoundation)

ZRef<ZNetscape::GuestFactory> ZNetscape::sMakeGuestFactory_MacPlugin(const std::string& iPath)
	{
	ZRef<ZNetscape::GuestFactory> result;

	#if ZCONFIG_SPI_Enabled(CoreFoundation)
		CFStringRef thePath = ::CFStringCreateWithCString(
			nil, iPath.c_str(), kCFStringEncodingUTF8);
		if (thePath)
			{
			CFURLRef theURL = ::CFURLCreateWithFileSystemPath(
				nil, thePath, kCFURLPOSIXPathStyle, true);
			::CFRelease(thePath);
			if (theURL)
				{
				CFPlugInRef thePlugInRef = ::CFPlugInCreate(nil, theURL);
				::CFRelease(theURL);
				if (thePlugInRef)
					{
					result = sMakeGuestFactory_MacPlugin(thePlugInRef);
					::CFRelease(thePlugInRef);
					}
				}
			}
	#endif

	return result;
	}
