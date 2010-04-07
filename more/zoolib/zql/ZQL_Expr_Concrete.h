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

#ifndef __ZQL_Expr_Concrete__
#define __ZQL_Expr_Concrete__ 1
#include "zconfig.h"

#include "zoolib/zql/ZQL_Expr_Relation.h"

NAMESPACE_ZOOLIB_BEGIN
namespace ZQL {

// =================================================================================================
#pragma mark -
#pragma mark * ExprRep_Concrete

class ExprRep_Concrete : public ExprRep_Relation
	{
protected:
	ExprRep_Concrete();

public:
	virtual ~ExprRep_Concrete();

// From ZExprRep_Relation
	virtual bool Accept(Visitor_ExprRep_Relation& iVisitor);
	};

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_ExprRep_Concrete

class Visitor_ExprRep_Concrete : public virtual Visitor_ExprRep_Relation
	{
public:
	virtual bool Visit_Concrete(ZRef<ExprRep_Concrete> iRep);
	};

// =================================================================================================
#pragma mark -
#pragma mark * Expr_Concrete

class Expr_Concrete : public Expr_Relation
	{
	typedef Expr_Relation inherited;

	Expr_Concrete operator=(const Expr_Relation&);
	Expr_Concrete operator=(const ZRef<ExprRep_Relation>&);

public:
	Expr_Concrete();
	Expr_Concrete(const Expr_Concrete& iOther);
	~Expr_Concrete();
	Expr_Concrete& operator=(const Expr_Concrete& iOther);

	Expr_Concrete(const ZRef<ExprRep_Concrete>& iRep);

	operator ZRef<ExprRep_Concrete>() const;
	};

} // namespace ZQL
NAMESPACE_ZOOLIB_END

#endif // __ZQL_Expr_Concrete__
