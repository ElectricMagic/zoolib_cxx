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

#ifndef __ZQE_Row__
#define __ZQE_Row__ 1
#include "zconfig.h"

#include "zoolib/ZCounted.h"
#include "zoolib/ZVal_Any.h"

#include <set>

namespace ZooLib {
namespace ZQE {

// =================================================================================================
#pragma mark -
#pragma mark * Row

class Row : public ZCounted
	{
protected:
	Row();

public:
	virtual ~Row();

// Our protocol
	virtual size_t Count() = 0;
	virtual ZVal_Any Get(size_t iIndex) = 0;

	virtual void GetAnnotations(std::set<ZRef<ZCounted> >& ioAnnotations);
	};

// =================================================================================================
#pragma mark -
#pragma mark * RowVector

class RowVector : public ZCounted
	{
public:
	RowVector();
	RowVector(const std::vector<ZRef<Row> >& iRows);
	RowVector(std::vector<ZRef<Row> >* ioRows);
	virtual ~RowVector();

// Our protocol
	size_t Count();
	ZRef<Row> Get(size_t iIndex);

	const std::vector<ZRef<Row> >& GetRows();

private:
	std::vector<ZRef<Row> > fRows;
	};

// =================================================================================================
#pragma mark -
#pragma mark * Row_Pair

class Row_Pair : public Row
	{
public:
	Row_Pair(const ZRef<Row>& iLeft, const ZRef<Row>& iRight);
	virtual ~Row_Pair();

// From Row
	virtual size_t Count();
	virtual ZVal_Any Get(size_t iIndex);

	virtual void GetAnnotations(std::set<ZRef<ZCounted> >& ioAnnotations);

private:
	const ZRef<Row> fLeft;
	const ZRef<Row> fRight;
	};

// =================================================================================================
#pragma mark -
#pragma mark * Row_Vector

class Row_Vector : public Row
	{
public:
	Row_Vector(const std::vector<ZVal_Any>& iVals);
	Row_Vector(std::vector<ZVal_Any>* ioVals);
	virtual ~Row_Vector();

// From Row
	virtual size_t Count();
	virtual ZVal_Any Get(size_t iIndex);

	virtual void GetAnnotations(std::set<ZRef<ZCounted> >& ioAnnotations);

// Our protocol
	void AddAnnotation(ZRef<ZCounted> iCounted);
	void AddAnnotations(const std::set<ZRef<ZCounted> >& iAnnotations);

private:
	std::vector<ZVal_Any> fVals;
	std::set<ZRef<ZCounted> > fAnnotations;
	};

} // namespace ZQE
} // namespace ZooLib

#endif // __ZQE_Row__
