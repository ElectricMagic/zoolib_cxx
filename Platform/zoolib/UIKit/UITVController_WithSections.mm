// Copyright (c) 2010 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/UIKit/UITVController_WithSections.h"

#if ZCONFIG_SPI_Enabled(iPhone)

#include "zoolib/Apple/Callable_ObjC.h"

#include "zoolib/Log.h"
#include "zoolib/Util_STL_map.h"
#include "zoolib/Util_STL_set.h"
#include "zoolib/ZMACRO_foreach.h"

#include "zoolib/Apple/ZP_NS.h"
#include "zoolib/Apple/Util_NS.h"

#import <UIKit/UIDevice.h>
#import <UIKit/UIGestureRecognizerSubclass.h>
#import <UIKit/UIScreen.h>

#import <QuartzCore/CATransaction.h>
#import <QuartzCore/CAShapeLayer.h>

namespace ZooLib {
namespace UIKit {

using std::map;
using std::pair;
using std::set;
using std::vector;

// =================================================================================================
#pragma mark - Helpers

static void spUpdatePopovers()
	{
	[[NSNotificationCenter defaultCenter]
		postNotificationName:@"UIPopoverControllerShouldMove"
		object:nil];
	}

NSArray* sMakeNSIndexPathArray(size_t iSectionIndex, size_t iBaseRowIndex, size_t iCount)
	{
	NSUInteger theIndices[2];
	theIndices[0] = iSectionIndex;
	NSMutableArray* theArray = [NSMutableArray arrayWithCapacity:iCount];
	for (size_t xx = 0; xx < iCount; ++xx)
		{
		theIndices[1] = iBaseRowIndex + xx;
		[theArray addObject:[NSIndexPath indexPathWithIndexes:&theIndices[0] length:2]];
		}
	return theArray;
	}

NSIndexSet* sMakeIndexSet(size_t iIndex)
	{ return [NSIndexSet indexSetWithIndex:iIndex]; }

// =================================================================================================
#pragma mark - Section

Section::Section(ZP<SectionBody> iBody)
:	fHideWhenEmpty(false)
,	fBody(iBody)
	{}

ZP<SectionBody> Section::GetBody()
	{ return fBody; }

bool Section::HideWhenEmpty()
	{ return fHideWhenEmpty; }

ZQ<CGFloat> Section::QHeaderHeight()
	{ return fHeaderHeightQ; }

ZQ<CGFloat> Section::QFooterHeight()
	{ return fFooterHeightQ; }

ZQ<string8> Section::QHeaderTitle()
	{ return fHeaderTitleQ; }

ZQ<string8> Section::QFooterTitle()
	{ return fFooterTitleQ; }

ZP<UIView> Section::QHeaderView()
	{ return fHeaderViewQ; }

ZP<UIView> Section::QFooterView()
	{ return fFooterViewQ; }

ZQ<UITableViewRowAnimation> Section::QSectionAnimation_Insert()
	{ return fSectionAnimation_InsertQ; }

ZQ<UITableViewRowAnimation> Section::QSectionAnimation_Delete()
	{ return fSectionAnimation_DeleteQ; }

ZQ<UITableViewRowAnimation> Section::QSectionAnimation_Reload()
	{ return fSectionAnimation_ReloadQ; }

UITableViewRowAnimation Section::SectionAnimation_Insert()
	{
	if (ZQ<UITableViewRowAnimation> theQ = this->QSectionAnimation_Insert())
		return *theQ;
	return UITableViewRowAnimationNone;
	}

UITableViewRowAnimation Section::SectionAnimation_Delete()
	{
	if (ZQ<UITableViewRowAnimation> theQ = this->QSectionAnimation_Delete())
		return *theQ;
	return UITableViewRowAnimationNone;
	}

UITableViewRowAnimation Section::SectionAnimation_Reload()
	{
	if (ZQ<UITableViewRowAnimation> theQ = this->QSectionAnimation_Reload())
		return *theQ;
	return UITableViewRowAnimationNone;
	}

// =================================================================================================
#pragma mark - SectionBody::RowMeta

SectionBody::RowMeta::RowMeta()
:	fBase(0)
,	fLimit(0)
	{}

void SectionBody::RowMeta::UpdateCount(size_t iCount)
	{
	fBase += fLimit;
	fLimit = iCount;
	}

// =================================================================================================
#pragma mark - SectionBody::RowUpdate

SectionBody::RowUpdate::RowUpdate(
	RowMeta& ioRowMeta, std::map<size_t, UITableViewRowAnimation>& ioMap)
:	fRowMeta(ioRowMeta)
,	fMap(ioMap)
	{}

void SectionBody::RowUpdate::Add(size_t iIndex, UITableViewRowAnimation iRowAnimation)
	{ this->AddRange(iIndex, 1, iRowAnimation); }

void SectionBody::RowUpdate::AddAll(UITableViewRowAnimation iRowAnimation)
	{ this->AddRange(0, fRowMeta.fLimit, iRowAnimation); }

void SectionBody::RowUpdate::AddRange(
	size_t iStart, size_t iCount, UITableViewRowAnimation iRowAnimation)
	{
	ZAssert(iStart <= fRowMeta.fLimit);
	if (iCount)
		{
		ZAssert(iStart + iCount <= fRowMeta.fLimit);
		while (iCount--)
			Util_STL::sInsertMust(0, fMap, fRowMeta.fBase + iStart++, iRowAnimation);
		}
	}

// =================================================================================================
#pragma mark - SectionBody

void SectionBody::ViewWillAppear(UITableView* iTV)
	{}

void SectionBody::ViewDidAppear(UITableView* iTV)
	{}

void SectionBody::ViewWillDisappear(UITableView* iTV)
	{}

void SectionBody::ViewDidDisappear(UITableView* iTV)
	{}

bool SectionBody::FindSectionBody(ZP<SectionBody> iSB, size_t& ioRow)
	{ return iSB == this; }

// =================================================================================================
#pragma mark - SectionBody_Concrete

SectionBody_Concrete::SectionBody_Concrete()
:	fRowAnimation_Insert(UITableViewRowAnimationRight)
,	fRowAnimation_Delete(UITableViewRowAnimationRight)
,	fRowAnimation_Reload(UITableViewRowAnimationNone)
,	fApplyAccessory(true)
	{}

bool SectionBody_Concrete::FindSectionBody(ZP<SectionBody> iSB, size_t& ioRow)
	{
	if (iSB == this)
		return true;
	ioRow += this->NumberOfRows();
	return false;
	}

void SectionBody_Concrete::ApplyAccessory(size_t iRowIndex, ZP<UITableViewCell> ioCell)
	{
	if ([ioCell accessoryView])
		{}
	else if (fCallable_ButtonTapped)
		{
		[ioCell setAccessoryType:UITableViewCellAccessoryDetailDisclosureButton];
		}
	else if (this->CanSelect(false, iRowIndex) | false)
		{
		[ioCell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
		}
	else
		{
		[ioCell setAccessoryType:UITableViewCellAccessoryNone];
		}

	if ([ioCell editingAccessoryView])
		{}
	else if (fCallable_ButtonTapped_Editing)
		{
		[ioCell setEditingAccessoryType:UITableViewCellAccessoryDetailDisclosureButton];
		}
	else if (this->CanSelect(true, iRowIndex) | false)
		{
		[ioCell setEditingAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
		}
	else
		{
		[ioCell setEditingAccessoryType:UITableViewCellAccessoryNone];
		}
	}

ZQ<UITableViewCellEditingStyle> SectionBody_Concrete::QEditingStyle(size_t iRowIndex)
	{ return fEditingStyleQ; }

bool SectionBody_Concrete::CommitEditingStyle(UITableViewCellEditingStyle iStyle, size_t iRowIndex)
	{ return false; }

ZQ<bool> SectionBody_Concrete::QShouldIndentWhileEditing(size_t iRowIndex)
	{ return fShouldIndentWhileEditingQ; }

ZQ<CGFloat> SectionBody_Concrete::QRowHeight(size_t iRowIndex)
	{ return fRowHeightQ; }

ZQ<NSInteger> SectionBody_Concrete::QIndentationLevel(size_t iRowIndex)
	{ return fIndentationLevelQ; }

bool SectionBody_Concrete::ButtonTapped(UITVHandler_WithSections* iTVC,
	UITableView* iTableView, NSIndexPath* iIndexPath, size_t iRowIndex)
	{
	if ([iTableView isEditing])
		{
		if (fCallable_ButtonTapped_Editing)
			{
			return fCallable_ButtonTapped_Editing->Call(
				iTVC, iTableView, iIndexPath, this, iRowIndex);
			}
		}
	else
		{
		if (fCallable_ButtonTapped)
			return fCallable_ButtonTapped->Call(iTVC, iTableView, iIndexPath, this, iRowIndex);
		}
	return false;
	}

bool SectionBody_Concrete::RowSelected(UITVHandler_WithSections* iTVC,
	UITableView* iTableView, NSIndexPath* iIndexPath, size_t iRowIndex)
	{
	if ([iTableView isEditing])
		{
		if (fCallable_RowSelected_Editing)
			{
			return fCallable_RowSelected_Editing->Call(
				iTVC, iTableView, iIndexPath, this, iRowIndex);
			}
		}
	else
		{
		if (fCallable_RowSelected)
			return fCallable_RowSelected->Call(iTVC, iTableView, iIndexPath, this, iRowIndex);
		}
	return false;
	}

ZQ<bool> SectionBody_Concrete::CanSelect(bool iEditing, size_t iRowIndex)
	{
	if (iEditing)
		{
		if (fCallable_RowSelected_Editing)
			return true;
		}
	else
		{
		if (fCallable_RowSelected)
			return true;
		}
	return false;
	}

UITableViewRowAnimation SectionBody_Concrete::RowAnimation_Insert()
	{ return fRowAnimation_Insert; }

UITableViewRowAnimation SectionBody_Concrete::RowAnimation_Delete()
	{ return fRowAnimation_Delete; }

UITableViewRowAnimation SectionBody_Concrete::RowAnimation_Reload()
	{ return fRowAnimation_Reload; }

// =================================================================================================
#pragma mark - SectionBody_SingleRow

SectionBody_SingleRow::SectionBody_SingleRow(ZP<UITableViewCell> iCell)
:	fCell_Pending(iCell)
	{
	ZAssert(not fCell_Pending || not [fCell_Pending reuseIdentifier]);
	}

size_t SectionBody_SingleRow::NumberOfRows()
	{
	if (fCell_Current)
		return 1;
	return 0;
	}

void SectionBody_SingleRow::PreUpdate()
	{
	ZAssert(not fCell_Pending || not [fCell_Pending reuseIdentifier]);
	fCell_New = fCell_Pending;
	}

bool SectionBody_SingleRow::WillBeEmpty()
	{ return not fCell_New; }

void SectionBody_SingleRow::Update_NOP()
	{}

void SectionBody_SingleRow::Update_Normal(RowMeta& ioRowMeta_Old, RowMeta& ioRowMeta_New,
	RowUpdate& ioRowUpdate_Insert, RowUpdate& ioRowUpdate_Delete, RowUpdate& ioRowUpdate_Reload)
	{
	if (fCell_Current)
		ioRowMeta_Old.UpdateCount(1);

	if (fCell_New)
		ioRowMeta_New.UpdateCount(1);

	if (fCell_Current)
		{
		if (fCell_New)
			{
			if (fCell_Current != fCell_New)
				{
				fCell_Current = fCell_New;
				ioRowUpdate_Reload.Add(0, this->RowAnimation_Reload());
				}
			}
		else
			{
			ioRowUpdate_Delete.Add(0, this->RowAnimation_Delete());
			}
		}
	else
		{
		if (fCell_New)
			ioRowUpdate_Insert.Add(0, this->RowAnimation_Insert());
		}
	}

void SectionBody_SingleRow::Update_Insert(RowMeta& ioRowMeta_New, RowUpdate& ioRowUpdate_New)
	{
	if (fCell_New)
		{
		ioRowMeta_New.UpdateCount(1);
		ioRowUpdate_New.Add(0, this->RowAnimation_Insert());
		}
	}

void SectionBody_SingleRow::Update_Delete(RowMeta& ioRowMeta_Old, RowUpdate& ioRowUpdate_Old)
	{
	if (fCell_Current)
		{
		ioRowMeta_Old.UpdateCount(1);
		ioRowUpdate_Old.Add(0, this->RowAnimation_Delete());
		}
	}

void SectionBody_SingleRow::FinishUpdate()
	{ fCell_Current = fCell_New; }

ZP<UITableViewCell> SectionBody_SingleRow::UITableViewCellForRow(
	UITableView* iView, size_t iRowIndex,
	bool& ioIsPreceded, bool& ioIsSucceeded)
	{
	if (fApplyAccessory)
		this->ApplyAccessory(0, fCell_Current);
	return fCell_Current;
	}

// =================================================================================================
#pragma mark - SectionBody_Multi

size_t SectionBody_Multi::NumberOfRows()
	{
	size_t count = 0;
	foreacha (aa, fBodies)
		count += aa->NumberOfRows();
	return count;
	}

void SectionBody_Multi::PreUpdate()
	{
	foreacha (aa, fBodies_Pending)
		aa->PreUpdate();
	}

bool SectionBody_Multi::WillBeEmpty()
	{
	foreacha (aa, fBodies_Pending)
		{
		if (not aa->WillBeEmpty())
			return false;
		}
	return true;
	}

void SectionBody_Multi::Update_NOP()
	{
	foreacha (aa, fBodies_Pending)
		aa->Update_NOP();
	}

void SectionBody_Multi::Update_Normal(
	RowMeta& ioRowMeta_Old, RowMeta& ioRowMeta_New,
	RowUpdate& ioRowUpdate_Insert, RowUpdate& ioRowUpdate_Delete, RowUpdate& ioRowUpdate_Reload)
	{
	const size_t endOld = fBodies.size();
	size_t iterNew = 0;
	const size_t endNew = fBodies_Pending.size();
	for (size_t iterOld = 0; iterOld < endOld; ++iterOld)
		{
		const ZP<SectionBody> bodyOld = fBodies[iterOld];
		const size_t inNew = find(fBodies_Pending.begin() + iterNew, fBodies_Pending.end(), bodyOld)
				- fBodies_Pending.begin();
		if (inNew == endNew)
			{
			// It's no longer in fBodies so delete its rows.
			bodyOld->Update_Delete(ioRowMeta_Old, ioRowUpdate_Delete);
			}
		else
			{
			// Insert any new bodies.
			while (iterNew < inNew)
				{
				ZP<SectionBody> theBody = fBodies_Pending[iterNew];
				theBody->Update_Insert(ioRowMeta_New, ioRowUpdate_Insert);
				++iterNew;
				}

			// It's still in fBodies. Give it the opportunity to apply any changes.
			bodyOld->Update_Normal(ioRowMeta_Old, ioRowMeta_New,
				ioRowUpdate_Insert, ioRowUpdate_Delete, ioRowUpdate_Reload);

			iterNew = inNew + 1;
			}
		}

	// Insert any remaining bodies.
	while (iterNew < endNew)
		{
		ZP<SectionBody> theBody = fBodies_Pending[iterNew];
		theBody->Update_Insert(ioRowMeta_New, ioRowUpdate_Insert);
		++iterNew;
		}
	}

void SectionBody_Multi::Update_Insert(RowMeta& ioRowMeta_New, RowUpdate& ioRowUpdate_New)
	{
	foreacha (aa, fBodies_Pending)
		aa->Update_Insert(ioRowMeta_New, ioRowUpdate_New);
	}

void SectionBody_Multi::Update_Delete(RowMeta& ioRowMeta_Old, RowUpdate& ioRowUpdate_Old)
	{
	foreacha (aa, fBodies_Pending)
		aa->Update_Delete(ioRowMeta_Old, ioRowUpdate_Old);
	}

void SectionBody_Multi::FinishUpdate()
	{
	fBodies = fBodies_Pending;
	foreacha (aa, fBodies)
		aa->FinishUpdate();
	}

void SectionBody_Multi::ViewWillAppear(UITableView* iTV)
	{
	foreacha (aa, fBodies)
		aa->ViewWillAppear(iTV);
	}

void SectionBody_Multi::ViewDidAppear(UITableView* iTV)
	{
	foreacha (aa, fBodies)
		aa->ViewDidAppear(iTV);
	}

void SectionBody_Multi::ViewWillDisappear(UITableView* iTV)
	{
	foreacha (aa, fBodies)
		aa->ViewWillDisappear(iTV);
	}

void SectionBody_Multi::ViewDidDisappear(UITableView* iTV)
	{
	foreacha (aa, fBodies)
		aa->ViewDidDisappear(iTV);
	}

bool SectionBody_Multi::FindSectionBody(ZP<SectionBody> iSB, size_t& ioRow)
	{
	foreacha (aa, fBodies)
		{
		if (aa->FindSectionBody(iSB, ioRow))
			return true;
		}
	return false;
	}

ZP<UITableViewCell> SectionBody_Multi::UITableViewCellForRow(
	UITableView* iView, size_t iRowIndex,
	bool& ioIsPreceded, bool& ioIsSucceeded)
	{
	if (iRowIndex != 0)
		ioIsPreceded = true;
	size_t localRowIndex;
	bool isSucceeded = false;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex, &isSucceeded))
		{
		bool localPreceded = false, localSucceeded = false;
		if (ZP<UITableViewCell> result =
			theBody->UITableViewCellForRow(iView, localRowIndex, localPreceded, localSucceeded))
			{
			if (localSucceeded || isSucceeded)
				ioIsSucceeded = true;
			return result;
			}
		}
	return null;
	}

ZQ<UITableViewCellEditingStyle> SectionBody_Multi::QEditingStyle(size_t iRowIndex)
	{
	size_t localRowIndex;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex))
		{
		if (ZQ<UITableViewCellEditingStyle> theQ = theBody->QEditingStyle(localRowIndex))
			return theQ;
		}
	return null;
	}

bool SectionBody_Multi::CommitEditingStyle(UITableViewCellEditingStyle iStyle, size_t iRowIndex)
	{
	size_t localRowIndex;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex))
		{
		if (theBody->CommitEditingStyle(iStyle, localRowIndex))
			return true;
		}
	return false;
	}

ZQ<bool> SectionBody_Multi::QShouldIndentWhileEditing(size_t iRowIndex)
	{
	size_t localRowIndex;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex))
		{
		if (ZQ<bool> theQ = theBody->QShouldIndentWhileEditing(localRowIndex))
			return theQ;
		}
	return null;
	}

ZQ<CGFloat> SectionBody_Multi::QRowHeight(size_t iRowIndex)
	{
	size_t localRowIndex;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex))
		{
		if (ZQ<CGFloat> theQ = theBody->QRowHeight(localRowIndex))
			return theQ;
		}
	return null;
	}

ZQ<NSInteger> SectionBody_Multi::QIndentationLevel(size_t iRowIndex)
	{
	size_t localRowIndex;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex))
		{
		if (ZQ<NSInteger> theQ = theBody->QIndentationLevel(localRowIndex))
			return theQ;
		}
	return null;
	}

bool SectionBody_Multi::ButtonTapped(UITVHandler_WithSections* iTVC,
	UITableView* iTableView, NSIndexPath* iIndexPath, size_t iRowIndex)
	{
	size_t localRowIndex;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex))
		{
		if (theBody->ButtonTapped(iTVC, iTableView, iIndexPath, localRowIndex))
			return true;
		}
	return false;
	}

bool SectionBody_Multi::RowSelected(UITVHandler_WithSections* iTVC,
	UITableView* iTableView, NSIndexPath* iIndexPath, size_t iRowIndex)
	{
	size_t localRowIndex;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex))
		{
		if (theBody->RowSelected(iTVC, iTableView, iIndexPath, localRowIndex))
			return true;
		}
	return false;
	}

ZQ<bool> SectionBody_Multi::CanSelect(bool iEditing, size_t iRowIndex)
	{
	size_t localRowIndex;
	if (ZP<SectionBody> theBody = this->pGetBodyAndRowIndex(iRowIndex, localRowIndex))
		{
		if (ZQ<bool> theQ = theBody->CanSelect(iEditing, localRowIndex))
			return theQ;
		}
	return null;
	}

ZP<SectionBody> SectionBody_Multi::pGetBodyAndRowIndex(size_t iIndex, size_t& oIndex)
	{ return this->pGetBodyAndRowIndex(iIndex, oIndex, nullptr); }

ZP<SectionBody> SectionBody_Multi::pGetBodyAndRowIndex(size_t iIndex, size_t& oIndex, bool* oIsSucceeded)
	{
	oIndex = iIndex;
	for (auto ii = fBodies.begin(); ii != fBodies.end(); ++ii)
		{
		ZP<SectionBody> theBody = *ii;
		const size_t theCount = theBody->NumberOfRows();
		if (oIndex < theCount)
			{
			if (oIsSucceeded)
				{
				++ii;
				while (ii != fBodies.end())
					{
					if ((*ii)->NumberOfRows())
						{
						*oIsSucceeded = true;
						break;
						}
					++ii;
					}
				}
			return theBody;
			}
		oIndex -= theCount;
		}
	return null;
	}

} // namespace UIKit
} // namespace ZooLib

// =================================================================================================
#pragma mark - UITVHandler_WithSections

using namespace ZooLib;
using namespace ZooLib::UIKit;

@interface UITVHandler_WithSections (Private)

- (ZP<Section>)pGetSection:(size_t)iSectionIndex;
- (void)pDoUpdate_WholeSections:(UITableView*)tableview;
- (void)pDoUpdate_Cells:(UITableView*)tableview;
- (void)pDoUpdate_Finish:(UITableView*)tableview;

@end // interface UITVHandler_WithSections

@implementation UITVHandler_WithSections

- (id)init
	{
	self = [super init];
	fTouchState = false;
	fNeedsUpdate = false;
	fUpdateInFlight = false;
	fCheckForUpdateQueued = false;
	fShown = false;
	return self;
	}

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView
	{ return fSections_Shown.size(); }

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section
	{
	if (ZP<Section> theSection = [self pGetSection:section])
		{
		if (ZQ<size_t> theQ = theSection->GetBody()->NumberOfRows())
			return *theQ;
		}
	return 0;
	}

- (void)tableView:(UITableView*)tableView
	commitEditingStyle:(UITableViewCellEditingStyle)editingStyle
	forRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		theSection->GetBody()->CommitEditingStyle(editingStyle, indexPath.row);
	}

- (BOOL)tableView:(UITableView *)tableView
	shouldHighlightRowAtIndexPath:(NSIndexPath *)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<bool> theQ = theSection->GetBody()->CanSelect([tableView isEditing], indexPath.row))
			{ return *theQ; }
		}
	return false;
	}

- (NSIndexPath*)tableView:(UITableView*)tableView
	willSelectRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<bool> theQ = theSection->GetBody()->CanSelect([tableView isEditing], indexPath.row))
			{
			if (not *theQ)
				return nil;
			}
		}
	return indexPath;
	}

 - (UITableViewCell*)tableView:(UITableView*)tableView
	cellForRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		{
		bool isPreceded = false, isSucceeded = false;
		if (ZP<UITableViewCell> theCell =
			theSection->GetBody()->UITableViewCellForRow(tableView, indexPath.row, isPreceded, isSucceeded))
			{
			return [theCell.Orphan() autorelease];
			}
		}

	return nullptr;
	}

- (CGFloat)tableView:(UITableView*)tableView heightForHeaderInSection:(NSInteger)section
	{
	if (ZP<Section> theSection = [self pGetSection:section])
		{
		if (ZQ<CGFloat> theQ = theSection->QHeaderHeight())
			return *theQ;
		}
	return tableView.sectionHeaderHeight;
	}

- (CGFloat)tableView:(UITableView*)tableView heightForFooterInSection:(NSInteger)section
	{
	if (ZP<Section> theSection = [self pGetSection:section])
		{
		if (ZQ<CGFloat> theQ = theSection->QFooterHeight())
			return *theQ;
		}
	return tableView.sectionFooterHeight;
	}

- (NSString*)tableView:(UITableView*)tableView titleForHeaderInSection:(NSInteger)section
	{
	if (ZP<Section> theSection = [self pGetSection:section])
		{
		if (ZQ<string8> theQ = theSection->QHeaderTitle())
			return Util_NS::sString(*theQ);
		}
	return nullptr;
	}

- (NSString*)tableView:(UITableView*)tableView titleForFooterInSection:(NSInteger)section
	{
	if (ZP<Section> theSection = [self pGetSection:section])
		{
		if (ZQ<string8> theQ = theSection->QFooterTitle())
			return Util_NS::sString(*theQ);
		}
	return nullptr;
	}

- (UIView*)tableView:(UITableView*)tableView viewForHeaderInSection:(NSInteger)section
	{
	if (ZP<Section> theSection = [self pGetSection:section])
		{
		if (ZP<UIView> theView = theSection->QHeaderView())
			return [theView.Orphan() autorelease];
		}
	return nullptr;
	}

- (UIView*)tableView:(UITableView*)tableView viewForFooterInSection:(NSInteger)section
	{
	if (ZP<Section> theSection = [self pGetSection:section])
		{
		if (ZP<UIView> theView = theSection->QFooterView())
			return [theView.Orphan() autorelease];
		}
	return nullptr;
	}

- (void)tableView:(UITableView*)tableView
	accessoryButtonTappedForRowWithIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		theSection->GetBody()->ButtonTapped(self, tableView, indexPath, indexPath.row);
	}

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		theSection->GetBody()->RowSelected(self, tableView, indexPath, indexPath.row);
	}

- (BOOL)tableView:(UITableView*)tableView
	shouldIndentWhileEditingRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<bool> theQ = theSection->GetBody()->QShouldIndentWhileEditing(indexPath.row))
			return *theQ;
		}
	return true;
	}

- (UITableViewCellEditingStyle)tableView:(UITableView*)tableView
	editingStyleForRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<UITableViewCellEditingStyle> theQ = theSection->GetBody()->QEditingStyle(indexPath.row))
			return *theQ;
		}
	return UITableViewCellEditingStyleNone;
	}

- (NSInteger)tableView:(UITableView*)tableView
	indentationLevelForRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<NSInteger> theQ = theSection->GetBody()->QIndentationLevel(indexPath.row))
			return *theQ;
		}
	return 0;
	}

- (void)doUpdateIfPossible:(UITableView*)tableView
	{
	fNeedsUpdate = true;

	if (sGetSet(fUpdateInFlight, true))
		return;

	[self pDoUpdate_WholeSections:tableView];
	}

-(void)pCheckForUpdate:(UITableView*)tableView
	{
	fCheckForUpdateQueued = false;

	if (fTouchState)
		return;

	if (not fNeedsUpdate)
		return;

	if (sGetSet(fUpdateInFlight, true))
		return;

	[self pDoUpdate_WholeSections:tableView];
	}

-(void)pEnqueueCheckForUpdate:(UITableView*)tableView
	{
	if (sGetSet(fCheckForUpdateQueued, true))
		return;

	[self
		performSelectorOnMainThread:@selector(pCheckForUpdate:)
		withObject:tableView
		waitUntilDone:NO];
	}

- (void)needsUpdate:(UITableView*)tableView
	{
	ZAssert(tableView);

	if (sGetSet(fNeedsUpdate, true))
		return;

	[self pEnqueueCheckForUpdate:tableView];
	}

- (NSIndexPath*)indexPathForSectionBody:(ZooLib::ZP<ZooLib::UIKit::SectionBody>)iSB
	{
	for (size_t theSection = 0; theSection < fSections_Shown.size(); ++theSection)
		{
		size_t theRow = 0;
		if (fSections_Shown[theSection]->GetBody()->FindSectionBody(iSB, theRow))
			return [NSIndexPath indexPathForRow:theRow inSection:theSection];
		}
	return nil;
	}

- (void)tableViewWillAppear:(UITableView*)tableView
	{
	[(UITableView_WithSections*)tableView deselect];
	for (size_t xx = 0; xx < fSections_All.size(); ++xx)
		fSections_All[xx]->GetBody()->ViewWillAppear(tableView);
	}

- (void)tableViewDidAppear:(UITableView*)tableView
	{
	fShown = true;
	[tableView flashScrollIndicators];
	for (size_t xx = 0; xx < fSections_All.size(); ++xx)
		fSections_All[xx]->GetBody()->ViewDidAppear(tableView);
	}

- (void)tableViewWillDisappear:(UITableView*)tableView
	{
	fShown = false;
	for (size_t xx = 0; xx < fSections_All.size(); ++xx)
		fSections_All[xx]->GetBody()->ViewWillDisappear(tableView);
	}

- (void)tableViewDidDisappear:(UITableView*)tableView
	{
	[(UITableView_WithSections*)tableView deselect];
	for (size_t xx = 0; xx < fSections_All.size(); ++xx)
		fSections_All[xx]->GetBody()->ViewDidDisappear(tableView);
	}

- (void)changeTouchState:(BOOL)touchState forTableView:(UITableView*)tableView
	{
	if (touchState)
		fTouchState = true;
	else if (sGetSet(fTouchState, false))
		[self pEnqueueCheckForUpdate:tableView];
	}

static void spInsertSections(UITableView* iTableView,
	size_t iBaseIndex,
	const ZP<Section>* iSections,
	size_t iCount,
	set<ZP<Section> >& ioSections_ToIgnore)
	{
	for (size_t xx = 0; xx < iCount; ++xx)
		{
		ZP<Section> theSection = iSections[xx];
		theSection->GetBody()->Update_NOP();
		theSection->GetBody()->FinishUpdate();
		ioSections_ToIgnore.insert(theSection);
		[iTableView
			insertSections:sMakeIndexSet(iBaseIndex + xx)
			withRowAnimation:theSection->SectionAnimation_Insert()];
		}
	}

- (void)pDoUpdate_WholeSections:(UITableView*)tableView
	{
	ZAssert(tableView);

	if (not sGetSet(fNeedsUpdate, false))
		return;

	ZAssert(fUpdateInFlight);

	ZLOGF(w, eDebug + 1);

	fSections_ToIgnore.clear();
	std::vector<ZooLib::ZP<ZooLib::UIKit::Section> > sections_Pending;
	for (size_t xx = 0; xx < fSections_All.size(); ++xx)
		{
		ZP<Section> theSection = fSections_All[xx];
		theSection->GetBody()->PreUpdate();
		if (not theSection->HideWhenEmpty() || not theSection->GetBody()->WillBeEmpty())
			sections_Pending.push_back(theSection);
		}

	// We've done PreUpdate on every section, and sections_Pending contains
	// those sections that will be visible.

	if (not fShown)
		{
		// We're not onscreen, so we can Update/Finish all sections, switch
		// to the new list of sections, reloadData, check for pending updates and return.
		for (size_t xx = 0; xx < fSections_All.size(); ++xx)
			{
			ZP<Section> theSection = fSections_All[xx];
			theSection->GetBody()->Update_NOP();
			theSection->GetBody()->FinishUpdate();
			}
		fSections_Shown = sections_Pending;
		[tableView reloadData];
		fUpdateInFlight = false;
		if (fNeedsUpdate)
			[self pEnqueueCheckForUpdate:tableView];
		return;
		}

	if (fSections_Shown == sections_Pending)
		{
		// The list of sections hasn't changed so move directly on to pDoUpdate_Cells
		[self pDoUpdate_Cells:tableView];
		return;
		}

	// We need to insert and remove sections.
	[CATransaction begin];

	[CATransaction setCompletionBlock:
		^{
		// On completion we push the invocation of pDoUpdate_Cells onto the main queue, so that it
		// is invoked from the main queue, not from the completion.
		dispatch_async
			(
			dispatch_get_main_queue(),
				^{
				[self pDoUpdate_Cells:tableView];
				}
			);
		}
	];

	[tableView beginUpdates];

	const vector<ZP<Section> > sectionsOld = fSections_Shown;
	fSections_Shown = sections_Pending;

	const size_t endOld = sectionsOld.size();

	size_t iterNew = 0;
	const size_t endNew = fSections_Shown.size();

	for (size_t iterOld = 0; iterOld < endOld; ++iterOld)
		{
		const ZP<Section> sectionOld = sectionsOld[iterOld];
		const size_t inNew = find(fSections_Shown.begin(), fSections_Shown.end(), sectionOld)
			- fSections_Shown.begin();
		if (inNew == endNew)
			{
			w << "\n" << "Delete section: " << iterOld;
			// sectionOld is no longer in fSections_Shown so must be deleted.
			[tableView
				deleteSections:sMakeIndexSet(iterOld)
				withRowAnimation:sectionOld->SectionAnimation_Delete()];
			// But it does need to be update/finished
			sectionOld->GetBody()->Update_NOP();
			sectionOld->GetBody()->FinishUpdate();
			}
		else
			{
			// sectionOld is still present. Don't do anything with it for now -- any cell
			// reload/insert/delete will happen later.
			if (size_t countToInsert = inNew - iterNew)
				{
				// There are sections to insert prior to sectionOld.
				w << "\n" << "Insert sections: " << iterNew << " to " << iterNew + countToInsert;
				spInsertSections(tableView,
					iterNew, &fSections_Shown[iterNew], countToInsert, fSections_ToIgnore);
				}
			iterNew = inNew + 1;
			}
		}

	// Insert remainder of pending.
	if (size_t countToInsert = endNew - iterNew)
		{
		w << "\n" << "Insert sections (2nd): " << iterNew << " to " << iterNew + countToInsert;
		spInsertSections(tableView,
			iterNew, &fSections_Shown[iterNew], countToInsert, fSections_ToIgnore);
		}

	[tableView endUpdates];

	[CATransaction commit];
	}

- (void)pDoUpdate_Cells:(UITableView*)tableView
	{
	ZAssert(tableView);
	ZAssert(fUpdateInFlight);

	// We really must be shown, otherwise pDoUpdate_WholeSections would not have called us.
	ZAssert(fShown);

	vector<map<size_t, UITableViewRowAnimation> > theInserts(fSections_Shown.size());
	vector<map<size_t, UITableViewRowAnimation> > theDeletes(fSections_Shown.size());
	vector<map<size_t, UITableViewRowAnimation> > theReloads(fSections_Shown.size());

	bool anyChanges = false;
	for (size_t xx = 0; xx < fSections_Shown.size(); ++xx)
		{
		if (not Util_STL::sContains(fSections_ToIgnore, fSections_Shown[xx]))
			{
			SectionBody::RowMeta theRowMeta_Old;
			SectionBody::RowMeta theRowMeta_New;
			SectionBody::RowUpdate theRowUpdate_Reload(theRowMeta_Old, theReloads[xx]);
			SectionBody::RowUpdate theRowUpdate_Delete(theRowMeta_Old, theDeletes[xx]);
			SectionBody::RowUpdate theRowUpdate_Insert(theRowMeta_New, theInserts[xx]);
			fSections_Shown[xx]->GetBody()->Update_Normal(theRowMeta_Old, theRowMeta_New,
				theRowUpdate_Insert, theRowUpdate_Delete, theRowUpdate_Reload);

			if (theReloads[xx].size() || theDeletes[xx].size() || theInserts[xx].size())
				anyChanges = true;
			}
		}
	fSections_ToIgnore.clear();

	if (anyChanges)
		{
		ZLOGF(w, eDebug + 1);
		[CATransaction begin];

		[CATransaction setCompletionBlock:^{ [self pDoUpdate_Finish:tableView]; }];

		[tableView beginUpdates];

		for (size_t xx = 0; xx < theReloads.size(); ++xx)
			{
			map<size_t, UITableViewRowAnimation>& theMap = theReloads[xx];
			foreacha (entry, theMap)
				{
				w << "\n" << xx << "    " << entry.first << "   " << entry.second;
				[tableView
					reloadRowsAtIndexPaths:sMakeNSIndexPathArray(xx, entry.first, 1)
					withRowAnimation:entry.second];
				}
			}


		w << "\n" << "Deletes:";
		for (size_t xx = 0; xx < theDeletes.size(); ++xx)
			{
			map<size_t, UITableViewRowAnimation>& theMap = theDeletes[xx];
			foreacha (entry, theMap)
				{
				w << "\n" << xx << "    " << entry.first << "   " << entry.second;
				[tableView
					deleteRowsAtIndexPaths:sMakeNSIndexPathArray(xx, entry.first, 1)
					withRowAnimation:entry.second];
				}
			}

		w << "\n" << "Inserts:";
		for (size_t xx = 0; xx < theInserts.size(); ++xx)
			{
			map<size_t, UITableViewRowAnimation>& theMap = theInserts[xx];
			foreacha (entry, theMap)
				{
				w << "\n" << xx << "    " << entry.first << "   " << entry.second;
				[tableView
					insertRowsAtIndexPaths:sMakeNSIndexPathArray(xx, entry.first, 1)
					withRowAnimation:entry.second];
				}
			}

		for (size_t xx = 0; xx < fSections_All.size(); ++xx)
			fSections_All[xx]->GetBody()->FinishUpdate();

		[tableView endUpdates];

		[CATransaction commit];
		}
	else
		{
		ZLOGTRACE(eDebug + 1);
		for (size_t xx = 0; xx < fSections_All.size(); ++xx)
			fSections_All[xx]->GetBody()->FinishUpdate();

		[self pDoUpdate_Finish:tableView];
		}
	}

- (void)pDoUpdate_Finish:(UITableView*)tableView
	{
	ZLOGTRACE(eDebug + 1);
	ZAssert(tableView);
	ZAssert(fUpdateInFlight);
	fUpdateInFlight = false;

	spUpdatePopovers();
	if (fNeedsUpdate)
		[self pEnqueueCheckForUpdate:tableView];
	}

-(ZP<Section>)pGetSection:(size_t)iSectionIndex
	{
	if (iSectionIndex < fSections_Shown.size())
		return fSections_Shown[iSectionIndex];
	return null;
	}

@end // implementation UITVHandler_WithSections

// =================================================================================================
#pragma mark - UITVHandler_WithSections_VariableRowHeight

@implementation UITVHandler_WithSections_VariableRowHeight

- (CGFloat)tableView:(UITableView*)tableView heightForRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZP<Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<CGFloat> theQ = theSection->GetBody()->QRowHeight(indexPath.row))
			return *theQ;
		}
	return tableView.rowHeight;
	}

@end // implementation UITVHandler_WithSections_VariableRowHeight

// =================================================================================================
#pragma mark - UIGestureRecognizer_TableViewWithSections

@interface UIGestureRecognizer_TableViewWithSections : UIGestureRecognizer
	{
@public
	UITableView_WithSections* fTV;
	bool fCallEnd;
	}

@end // interface UIGestureRecognizer_TableViewWithSections

@implementation UIGestureRecognizer_TableViewWithSections

- (id)init
	{
	self = [super init];
	fCallEnd = false;
	self.cancelsTouchesInView = NO;
	self.delaysTouchesEnded = NO;
	return self;
	}

- (void)reset
	{
	[super reset];
	if (sGetSet(fCallEnd, false))
		[fTV pChangeTouchState:NO];
	}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
	{
	[super touchesBegan:touches withEvent:event];
	if (not sGetSet(fCallEnd, true))
		[fTV pChangeTouchState:YES];
	}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
	{
	[super touchesMoved:touches withEvent:event];
	}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
	{
	[super touchesEnded:touches withEvent:event];
	self.state = UIGestureRecognizerStateFailed;
	if (sGetSet(fCallEnd, false))
		[fTV pChangeTouchState:NO];
	}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
	{
	[super touchesCancelled:touches withEvent:event];
	self.state = UIGestureRecognizerStateFailed;
	if (sGetSet(fCallEnd, false))
		[fTV pChangeTouchState:NO];
	}

@end // UIGestureRecognizer_TableViewWithSections

// =================================================================================================
#pragma mark - UITableView_WithSections

@implementation UITableView_WithSections

- (id)initWithFrame:(CGRect)frame style:(UITableViewStyle)style variableRowHeight:(BOOL)variableRowHeight
	{
	self = [super initWithFrame:frame style:style];

	fCallable_NeedsUpdate = sCallable<void()>(self, @selector(needsUpdate));

	if (variableRowHeight)
		fHandler = sAdopt& [[UITVHandler_WithSections_VariableRowHeight alloc] init];
	else
		fHandler = sAdopt& [[UITVHandler_WithSections alloc] init];
	[self setDelegate:fHandler];
	[self setDataSource:fHandler];
	ZP<UIGestureRecognizer_TableViewWithSections> theGR = sAdopt&
		[[UIGestureRecognizer_TableViewWithSections alloc] init];
	theGR->fTV = self;
	[self addGestureRecognizer:theGR];
	return self;
	}

- (id)initWithFrame:(CGRect)frame style:(UITableViewStyle)style
	{ return [self initWithFrame:frame style:style variableRowHeight:NO]; }

- (void)doUpdateIfPossible
	{
	if (fHandler)
		[fHandler doUpdateIfPossible:self];
	}

- (void)needsUpdate
	{
	if (fHandler)
		[fHandler needsUpdate:self];
	}

- (void)appendSection:(ZooLib::ZP<ZooLib::UIKit::Section>) iSection
	{
	fHandler->fSections_All.push_back(iSection);
	[self needsUpdate];
	}

- (NSIndexPath*)indexPathForSectionBody:(ZooLib::ZP<ZooLib::UIKit::SectionBody>)iSB
	{ return [fHandler indexPathForSectionBody:iSB]; }

- (void)deselect
	{
	if (NSIndexPath* thePath = [self indexPathForSelectedRow])
		[self deselectRowAtIndexPath:thePath animated:YES];
	}

- (void)pChangeTouchState:(BOOL)touchState
	{ [fHandler changeTouchState:touchState forTableView:self]; }

@end // implementation UITableView_WithSections

#endif // ZCONFIG_SPI_Enabled(iPhone)
