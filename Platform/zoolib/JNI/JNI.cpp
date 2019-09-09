/* -------------------------------------------------------------------------------------------------
Copyright (c) 2015 Andrew Green
http://www.zoolib.org

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,Publish, distribute,
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

#include "zoolib/JNI/JNI.h"

#include <stdexcept>

namespace ZooLib {
namespace JNI {

// =================================================================================================
#pragma mark - JNI::EnsureAttachedToCurrentThread

EnsureAttachedToCurrentThread::EnsureAttachedToCurrentThread(JavaVM* iJavaVM)
:	fJavaVM(iJavaVM)
,	fNeedsDetach(false)
	{
	JNIEnv* env;
	const jint result = fJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
	switch (result)
		{
		case JNI_OK:
			{
			// Already attached, get env into our thread-local Env.
			break;
			}
		case JNI_EDETACHED:
			{
			// Not attached, we'll need to detach on destruction.
			JavaVMAttachArgs args;
			args.version = JNI_VERSION_1_6;
			args.name = nullptr;
			args.group = nullptr;
			if (0 != fJavaVM->AttachCurrentThread(&env, &args))
				{
				throw std::runtime_error("EnsureAttachedToCurrentThread failed, couldn't attach");
				}
			fNeedsDetach = true;
			break;
			}
		default:
			{
			throw std::runtime_error("EnsureAttachedToCurrentThread failed, other");
			}
		}

	// If we get to here, then env holds the env for the current thread, whether it was
	// because we created it, or because it already existed. So shove it into our fEnv.
	fEnv.Set(env);
	}

EnsureAttachedToCurrentThread::~EnsureAttachedToCurrentThread()
	{
	if (fNeedsDetach)
		fJavaVM->DetachCurrentThread();
	}

// =================================================================================================
#pragma mark - PushPopLocalFrame

PushPopLocalFrame::PushPopLocalFrame()
:	fEnv(EnvTV::sGet())
	{
	assert(fEnv);
	fEnv->PushLocalFrame(50);
	}

PushPopLocalFrame::PushPopLocalFrame(JNIEnv* iEnv)
:	fEnv(iEnv)
	{
	assert(fEnv);
	fEnv->PushLocalFrame(50);
	}

PushPopLocalFrame::~PushPopLocalFrame()
	{
	fEnv->PopLocalFrame(nullptr);
	}

// =================================================================================================
#pragma mark - sAsString

std::string sAsString(jstring s)
	{ return sAsString(EnvTV::sGet(), s); }

std::string sAsString(JNIEnv *env, jstring s)
	{
	std::string result;
	if (env && s)
		{
		if (const char* charP = env->GetStringUTFChars(s, 0))
			{
			result = charP;
			env->ReleaseStringUTFChars(s, charP);
			}
		}
	return result;
	}

} // namespace JNI
} // namespace ZooLib
