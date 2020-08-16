// Copyright (c) 2010 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_RelationalAlgebra_Expr_Rel_Rename_h__
#define __ZooLib_RelationalAlgebra_Expr_Rel_Rename_h__ 1
#include "zconfig.h"

#include "zoolib/Expr/Expr_Op_T.h"
#include "zoolib/RelationalAlgebra/Expr_Rel.h"

#include <string>

namespace ZooLib {
namespace RelationalAlgebra {

class Visitor_Expr_Rel_Rename;

// =================================================================================================
#pragma mark - Expr_Rel_Rename

class Expr_Rel_Rename
:	public virtual Expr_Rel
,	public virtual Expr_Op1_T<Expr_Rel>
	{
	typedef Expr_Op1_T<Expr_Rel> inherited;
public:
	Expr_Rel_Rename(const ZP<Expr_Rel>& iOp0, const ColName& iNew, const ColName& iOld);
	virtual ~Expr_Rel_Rename();

// From Visitee
	virtual void Accept(const Visitor& iVisitor);

// From Expr_Op1_T<Expr_Rel>
	virtual void Accept_Expr_Op1(Visitor_Expr_Op1_T<Expr_Rel>& iVisitor);

	virtual ZP<Expr_Rel> Self();
	virtual ZP<Expr_Rel> Clone(const ZP<Expr_Rel>& iOp0);

// Our protocol
	virtual void Accept_Expr_Rel_Rename(Visitor_Expr_Rel_Rename& iVisitor);

	const ColName& GetNew() const;
	const ColName& GetOld() const;

private:
	const ColName fNew;
	const ColName fOld;
	};

// =================================================================================================
#pragma mark - Visitor_Expr_Rel_Rename

class Visitor_Expr_Rel_Rename
:	public virtual Visitor_Expr_Op1_T<Expr_Rel>
	{
public:
	virtual void Visit_Expr_Rel_Rename(const ZP<Expr_Rel_Rename>& iExpr);
	};

// =================================================================================================
#pragma mark - Relational operators

ZP<Expr_Rel> sRename(const ZP<Expr_Rel>& iExpr,
	const ColName& iNewPropName, const ColName& iOldPropName);

} // namespace RelationalAlgebra

template <>
int sCompare_T(const RelationalAlgebra::Expr_Rel_Rename& iL,
	const RelationalAlgebra::Expr_Rel_Rename& iR);

} // namespace ZooLib

#endif // __ZooLib_RelationalAlgebra_Expr_Rel_Rename_h__
