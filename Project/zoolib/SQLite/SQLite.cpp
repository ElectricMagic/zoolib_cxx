// Copyright (c) 2010 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/SQLite/SQLite.h"

#include "zoolib/Data_ZZ.h"

#include <stdexcept>

namespace ZooLib {
namespace SQLite {

// =================================================================================================
#pragma mark - SQLite

DB::DB(const string8& iPath)
:	fDB(nullptr)
,	fAdopted(true)
	{
	if (SQLITE_OK != ::sqlite3_open(iPath.c_str(), &fDB))
		throw std::runtime_error(std::string(__FUNCTION__) + ", Couldn't open sqlite database");
	}

DB::DB(sqlite3* iDB, bool iAdopt)
:	fDB(iDB)
,	fAdopted(iAdopt)
	{}

DB::~DB()
	{
	if (fDB && fAdopted)
		::sqlite3_close(fDB);
	}

sqlite3* DB::GetDB()
	{ return fDB; }

// =================================================================================================
#pragma mark - Iter

// fPosition is one-based, so that zero can be a special value.
// fPosition == 0 is the initial state, referencing the first result
// but not knowing if that result is there.
// fPosition == 1 still references the first result, but sqlite3_step
// has been called, and fHasValue tells us if we've got a value.

Iter::Iter(ZP<DB> iDB, const string8& iSQL, uint64 iPosition)
:	fDB(iDB)
,	fSQL(iSQL)
,	fStmt(nullptr)
,	fHasValue(false)
,	fPosition(0)
	{
	::sqlite3_prepare_v2(fDB->GetDB(), fSQL.c_str(), fSQL.size(), &fStmt, nullptr);

	if (fStmt)
		{
		// See note above. If iPosition == 1 we're referencing the first result,
		// but if neither HasValue nor Get is ever called then we'll save ourselves
		// some work by not calling sqlite3_step yet.
		if (iPosition >= 1)
			{
			while (iPosition--)
				this->pAdvance();
			}
		}
	}

Iter::Iter(ZP<DB> iDB, const string8& iSQL)
:	fDB(iDB)
,	fSQL(iSQL)
,	fStmt(nullptr)
,	fHasValue(false)
,	fPosition(0)
	{ ::sqlite3_prepare_v2(fDB->GetDB(), fSQL.c_str(), fSQL.size(), &fStmt, nullptr); }

Iter::~Iter()
	{
	if (fStmt)
		::sqlite3_finalize(fStmt);
	}

ZP<Iter> Iter::Clone(bool iRewound)
	{
	if (fStmt)
		{
		if (iRewound)
			return new Iter(fDB, fSQL);
		return new Iter(fDB, fSQL, fPosition);
		}
	return this;
	}

void Iter::Rewind()
	{
	if (fStmt)
		{
		::sqlite3_reset(fStmt);
		fHasValue = false;
		fPosition = 0;
		}
	}

bool Iter::HasValue()
	{
	if (fStmt)
		{
		if (fPosition == 0)
			this->pAdvance();
		return fHasValue;
		}
	return false;
	}

void Iter::Advance()
	{
	if (fStmt)
		this->pAdvance();
	}

size_t Iter::Count()
	{
	if (fStmt)
		return ::sqlite3_column_count(fStmt);
	return 0;
	}

string8 Iter::NameOf(size_t iIndex)
	{
	if (fStmt)
		{
		if (const char* theName = ::sqlite3_column_name(fStmt, iIndex))
			return theName;
		}
	return string8();
	}

Any Iter::Get(size_t iIndex)
	{
	if (fStmt)
		{
		if (fPosition == 0)
			this->pAdvance();

		if (fHasValue)
			{
			switch (::sqlite3_column_type(fStmt, iIndex))
				{
				case SQLITE_INTEGER:
					{
					return sAny<int64>(::sqlite3_column_int64(fStmt, iIndex));
					}
				case SQLITE_FLOAT:
					{
					return sAny<double>(::sqlite3_column_double(fStmt, iIndex));
					}
				case SQLITE3_TEXT:
					{
					const unsigned char* theText = ::sqlite3_column_text(fStmt, iIndex);
					return sAnyCounted<string8>(
						(const char*)theText, ::sqlite3_column_bytes(fStmt, iIndex));
					}
				case SQLITE_BLOB:
					{
					const void* theData = ::sqlite3_column_blob(fStmt, iIndex);
					return sAny<Data_ZZ>(theData, ::sqlite3_column_bytes(fStmt, iIndex));
					}
				}
			}
		}
	return Any();
	}

void Iter::pAdvance()
	{
	ZAssert(fStmt);

	if ((fHasValue || fPosition == 0) && SQLITE_ROW == ::sqlite3_step(fStmt))
		{
		++fPosition;
		fHasValue = true;
		}
	else
		{
		fHasValue = false;
		}
	}

} // namespace SQLite
} // namespace ZooLib
