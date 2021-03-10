// Copyright (c) 2010 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_RelationalAlgebra_Expr_Rel_Restrict_h__
#define __ZooLib_RelationalAlgebra_Expr_Rel_Restrict_h__ 1
#include "zconfig.h"

#include "zoolib/Expr/Expr_Bool.h"
#include "zoolib/Expr/Expr_Op_T.h"
#include "zoolib/RelationalAlgebra/Expr_Rel.h"

namespace ZooLib {
namespace RelationalAlgebra {

class Visitor_Expr_Rel_Restrict;

// =================================================================================================
#pragma mark - Expr_Rel_Restrict

class Expr_Rel_Restrict
:	public virtual Expr_Rel
,	public virtual Expr_Op1_T<Expr_Rel>
	{
	typedef Expr_Op1_T<Expr_Rel> inherited;
public:
	Expr_Rel_Restrict(const ZP<Expr_Rel>& iOp0, const ZP<Expr_Bool>& iExpr_Bool);
	virtual ~Expr_Rel_Restrict();

// From Visitee
	virtual void Accept(const Visitor& iVisitor);

// From Expr
	virtual int Compare(const ZP<Expr>& iOther);

// From Expr_Op1_T<Expr_Rel>
	virtual void Accept_Expr_Op1(Visitor_Expr_Op1_T<Expr_Rel>& iVisitor);

	virtual ZP<Expr_Rel> Self();
	virtual ZP<Expr_Rel> Clone(const ZP<Expr_Rel>& iOp0);

// Our protocol
	virtual void Accept_Expr_Rel_Restrict(Visitor_Expr_Rel_Restrict& iVisitor);

	const ZP<Expr_Bool>& GetExpr_Bool() const;

private:
	const ZP<Expr_Bool> fExpr_Bool;
	};

// =================================================================================================
#pragma mark - Visitor_Expr_Rel_Restrict

class Visitor_Expr_Rel_Restrict
:	public virtual Visitor_Expr_Op1_T<Expr_Rel>
	{
public:
	virtual void Visit_Expr_Rel_Restrict(const ZP<Expr_Rel_Restrict>& iExpr);
	};

// =================================================================================================
#pragma mark - Relational operators

ZP<Expr_Rel_Restrict> sRestrict(
	const ZP<Expr_Rel>& iExpr_Rel, const ZP<Expr_Bool>& iExpr_Bool);

ZP<Expr_Rel> operator&(
	const ZP<Expr_Rel>& iExpr_Rel, const ZP<Expr_Bool>& iExpr_Bool);

ZP<Expr_Rel> operator&(
	const ZP<Expr_Bool>& iExpr_Bool, const ZP<Expr_Rel>& iExpr_Rel);

ZP<Expr_Rel>& operator&=(ZP<Expr_Rel>& ioExpr_Rel, const ZP<Expr_Bool>& iExpr_Bool);

} // namespace RelationalAlgebra
} // namespace ZooLib

#endif // __ZooLib_RelationalAlgebra_Expr_Rel_Restrict_h__
