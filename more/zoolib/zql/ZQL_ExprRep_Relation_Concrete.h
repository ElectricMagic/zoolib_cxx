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

#ifndef __ZQL_ExprRep_Relation_Concrete__
#define __ZQL_ExprRep_Relation_Concrete__ 1
#include "zconfig.h"

#include "zoolib/zql/ZQL_ExprRep_Relation.h"

NAMESPACE_ZOOLIB_BEGIN
namespace ZQL {

// =================================================================================================
#pragma mark -
#pragma mark * ExprRep_Relation_Concrete

class ExprRep_Relation_Concrete : public ExprRep_Relation
	{
protected:
	ExprRep_Relation_Concrete();

public:
	virtual ~ExprRep_Relation_Concrete();

// From ExprRep_Relation
	virtual bool Accept_ExprRep_Relation(Visitor_ExprRep_Relation& iVisitor);
	};

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_ExprRep_Relation_Concrete

class Visitor_ExprRep_Relation_Concrete : public virtual Visitor_ExprRep_Relation
	{
public:
	virtual bool Visit_ExprRep_Relation_Concrete(ZRef<ExprRep_Relation_Concrete> iRep);
	};

} // namespace ZQL
NAMESPACE_ZOOLIB_END

#endif // __ZQL_ExprRep_Relation_Concrete__
