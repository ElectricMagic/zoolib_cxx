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

#ifndef __ZExpr_ValCondition__
#define __ZExpr_ValCondition__ 1
#include "zconfig.h"

#include "zoolib/ZExpr_ValCondition_T.h"
#include "zoolib/ZValCondition.h"

NAMESPACE_ZOOLIB_BEGIN

typedef ZExprRep_ValCondition_T<ZVal_Expr> ZExprRep_ValCondition;

typedef ZExpr_ValCondition_T<ZVal_Expr> ZExpr_ValCondition;

typedef ZVisitor_ExprRep_ValCondition_T<ZVal_Expr> ZVisitor_ExprRep_ValCondition;

inline ZRelHead sGetRelHead(const ZRef<ZExprRep_Logic>& iRep)
	{ return sGetRelHead_T<ZVal_Expr>(iRep); }

inline bool sMatches(const ZRef<ZExprRep_Logic>& iRep, const ZVal_Expr& iVal)
	{ return sMatches_T<ZVal_Expr>(iRep, iVal); }

inline bool sMatches(const ZValCondition& iValCondition, const ZVal_Expr& iVal)
	{ return sMatches_T<ZVal_Expr>(iValCondition, iVal); }

NAMESPACE_ZOOLIB_END

#endif // __ZExpr_ValCondition__
