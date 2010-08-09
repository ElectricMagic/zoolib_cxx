/* -------------------------------------------------------------------------------------------------
Copyright (c) 2010 Andrew Green
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

#include "zoolib/dataspace/ZDataspace_Util_Strim.h"
#include "zoolib/ZVisitor_Expr_Logic_ValPred_DoToStrim.h"

#include "zoolib/zra/ZRA_Util_Strim_RelHead.h"

namespace ZooLib {
namespace ZDataspace {

using ZRA::NameMap;

using std::set;
using std::vector;

// =================================================================================================
#pragma mark -
#pragma mark *

const ZStrimW& operator<<(const ZStrimW& w, const SearchSpec& iSearchSpec)
	{
	w << "(";
	bool isSubsequent = false;
	for (vector<NameMap>::const_iterator i = iSearchSpec.fNameMaps.begin();
		i != iSearchSpec.fNameMaps.end(); ++i)
		{
		if (isSubsequent)
			w << ", ";
		isSubsequent = true;
		w << *i;
		}
	w << ")\n";
	ZVisitor_Expr_Logic_ValPred_DoToStrim()
		.DoToStrim(ZVisitor_DoToStrim::Options(), w, sAsExpr_Logic(iSearchSpec.fPredCompound));
	return w;
	}

const ZStrimW& operator<<(const ZStrimW& w, const set<RelHead>& iSet)
	{
	bool isSubsequent = false;
	for (set<RelHead>::const_iterator i = iSet.begin(); i != iSet.end(); ++i)
		{
		if (isSubsequent)
			w << ", ";
		w << *i;
		}
	return w;
	}


} // namespace ZDataspace
} // namespace ZooLib
