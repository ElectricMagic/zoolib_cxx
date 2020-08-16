// Copyright (c) 2010 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/RelationalAlgebra/Expr_Rel_Product.h"

#include "zoolib/Compare_Ref.h"

namespace ZooLib {

// =================================================================================================
#pragma mark - sCompare_T

template <>
int sCompare_T(const RelationalAlgebra::Expr_Rel_Product& iL,
	const RelationalAlgebra::Expr_Rel_Product& iR)
	{
	if (int compare = sCompare_T(iL.GetOp0(), iR.GetOp0()))
		return compare;

	return sCompare_T(iL.GetOp1(), iR.GetOp1());
	}

ZMACRO_CompareRegistration_T(RelationalAlgebra::Expr_Rel_Product)

namespace RelationalAlgebra {

// =================================================================================================
#pragma mark - Expr_Rel_Product

Expr_Rel_Product::Expr_Rel_Product(const ZP<Expr_Rel>& iOp0, const ZP<Expr_Rel>& iOp1)
:	inherited(iOp0, iOp1)
	{}

void Expr_Rel_Product::Accept(const Visitor& iVisitor)
	{
	if (Visitor_Expr_Rel_Product* theVisitor = sDynNonConst<Visitor_Expr_Rel_Product>(&iVisitor))
		this->Accept_Expr_Rel_Product(*theVisitor);
	else
		inherited::Accept(iVisitor);
	}

void Expr_Rel_Product::Accept_Expr_Op2(Visitor_Expr_Op2_T<Expr_Rel>& iVisitor)
	{
	if (Visitor_Expr_Rel_Product* theVisitor = sDynNonConst<Visitor_Expr_Rel_Product>(&iVisitor))
		this->Accept_Expr_Rel_Product(*theVisitor);
	else
		inherited::Accept_Expr_Op2(iVisitor);
	}

ZP<Expr_Rel> Expr_Rel_Product::Self()
	{ return this; }

ZP<Expr_Rel> Expr_Rel_Product::Clone(const ZP<Expr_Rel>& iOp0, const ZP<Expr_Rel>& iOp1)
	{ return new Expr_Rel_Product(iOp0, iOp1); }

void Expr_Rel_Product::Accept_Expr_Rel_Product(Visitor_Expr_Rel_Product& iVisitor)
	{ iVisitor.Visit_Expr_Rel_Product(this); }

// =================================================================================================
#pragma mark - Visitor_Expr_Rel_Product

void Visitor_Expr_Rel_Product::Visit_Expr_Rel_Product(const ZP<Expr_Rel_Product>& iExpr)
	{ this->Visit_Expr_Op2(iExpr); }

// =================================================================================================
#pragma mark - Relational operators

ZP<Expr_Rel> sProduct(const ZP<Expr_Rel>& iLHS, const ZP<Expr_Rel>& iRHS)
	{
	if (iLHS && iRHS)
		return new Expr_Rel_Product(iLHS, iRHS);
	sSemanticError("sProduct, LHS and/or RHS are null");
	return null;
	}

ZP<Expr_Rel> operator*(const ZP<Expr_Rel>& iLHS, const ZP<Expr_Rel>& iRHS)
	{ return sProduct(iLHS, iRHS); }

ZP<Expr_Rel>& operator*=(ZP<Expr_Rel>& ioLHS, const ZP<Expr_Rel>& iRHS)
	{ return ioLHS = ioLHS * iRHS; }

} // namespace RelationalAlgebra
} // namespace ZooLib
