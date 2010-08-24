#import "zoolib/iphone/ZooLib_UITableViewController.h"

#include "zoolib/ZRef_NSObject.h"
#include "zoolib/ZUtil_NSObject.h"

namespace ZooLib {
namespace IPhone {

// =================================================================================================
#pragma mark -
#pragma mark * UITVC_Section

void UITVC_Section::SetHeaderHeight(ZQ<CGFloat> iHeight)
	{ fHeaderHeight = iHeight; }

void UITVC_Section::SetFooterHeight(ZQ<CGFloat> iHeight)
	{ fFooterHeight = iHeight; }

void UITVC_Section::SetHeaderTitle(ZQ<string8> iTitle)
	{ fHeaderTitle = iTitle; }

void UITVC_Section::SetFooterTitle(ZQ<string8> iTitle)
	{ fFooterTitle = iTitle; }

void UITVC_Section::SetHeaderView(ZRef<UIView> iUIView)
	{ fHeaderView = iUIView; }

void UITVC_Section::SetFooterView(ZRef<UIView> iUIView)
	{ fFooterView = iUIView; }

size_t UITVC_Section::NumberOfRows()
	{ return 0; }

ZRef<UITableViewCell> UITVC_Section::UITableViewCellForRow(UITableView* iView, size_t iIndex)
	{ return null; }

ZQ<UITableViewCellEditingStyle> UITVC_Section::EditingStyle(size_t iIndex)
	{ return null; }

ZQ<CGFloat> UITVC_Section::RowHeight(size_t iIndex)
	{ return null; }

ZQ<CGFloat> UITVC_Section::HeaderHeight()
	{ return fHeaderHeight; }

ZQ<CGFloat> UITVC_Section::FooterHeight()
	{ return fFooterHeight; }

ZQ<string8> UITVC_Section::HeaderTitle()
	{ return fHeaderTitle; }

ZQ<string8> UITVC_Section::FooterTitle()
	{ return fFooterTitle; }

ZRef<UIView> UITVC_Section::HeaderView()
	{ return fHeaderView; }

ZRef<UIView> UITVC_Section::FooterView()
	{ return fFooterView; }

void UITVC_Section::AccessoryButtonTapped(size_t iIndex)
	{}

ZQ<bool> UITVC_Section::ShouldIndentWhileEditing(size_t iIndex)
	{ return null; }

// =================================================================================================
#pragma mark -
#pragma mark * UITVC_Section_WithRow

void UITVC_Section_WithRow::AddRow(ZRef<Row> iRow)
	{
	fRows.push_back(iRow);
	}

size_t UITVC_Section_WithRow::NumberOfRows()
	{
	return fRows.size();
	}

ZRef<UITableViewCell> UITVC_Section_WithRow::UITableViewCellForRow(UITableView* iView, size_t iIndex)
	{
	if (ZRef<Row> theRow = this->pGetRow(iIndex))
		return theRow->UITableViewCellForRow(iView);
	return UITVC_Section::UITableViewCellForRow(iView, iIndex);
	}

ZQ<UITableViewCellEditingStyle> UITVC_Section_WithRow::EditingStyle(size_t iIndex)
	{
	if (ZRef<Row> theRow = this->pGetRow(iIndex))
		return theRow->EditingStyle();
	return null;
	}

void UITVC_Section_WithRow::AccessoryButtonTapped(size_t iIndex)
	{
	if (ZRef<Row> theRow = this->pGetRow(iIndex))
		return theRow->AccessoryButtonTapped();
	}

ZQ<bool> UITVC_Section_WithRow::ShouldIndentWhileEditing(size_t iIndex)
	{
	if (ZRef<Row> theRow = this->pGetRow(iIndex))
		return theRow->ShouldIndentWhileEditing();
	return null;
	}

ZQ<CGFloat> UITVC_Section_WithRow::RowHeight(size_t iIndex)
	{
	if (ZRef<Row> theRow = this->pGetRow(iIndex))
		return theRow->RowHeight();
	return null;
	}

ZRef<UITVC_Section_WithRow::Row> UITVC_Section_WithRow::pGetRow(size_t iIndex)
	{
	if (iIndex < fRows.size())
		return fRows[iIndex];
	return null;
	}

// =================================================================================================
#pragma mark -
#pragma mark * UITVC_Section_WithRow

ZRef<UITableViewCell> UITVC_Section_WithRow::Row::UITableViewCellForRow(UITableView* iView)
	{ return null; }

ZQ<UITableViewCellEditingStyle> UITVC_Section_WithRow::Row::EditingStyle()
	{ return null; }

ZQ<bool> UITVC_Section_WithRow::Row::ShouldIndentWhileEditing()
	{ return null; }

ZQ<CGFloat> UITVC_Section_WithRow::Row::RowHeight()
	{ return null; }

void UITVC_Section_WithRow::Row::AccessoryButtonTapped()
	{
	if (fCallable)
		fCallable->Call(this);
	}

} // namespace IPhone
} // namespace ZooLib

// =================================================================================================
#pragma mark -
#pragma mark * ZooLib_UITVC_WithSections

using ZooLib::IPhone::UITVC_Section;

@implementation ZooLib_UITVC_WithSections

// From UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView
	{ return fSections.size(); }

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:section])
		return theSection->NumberOfRows();
	return 0;
	}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
	{
	// The fact that this method is implemented is what enables "swipe to delete".
	}
 
 - (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZRef<UITableViewCell> theCell = theSection->UITableViewCellForRow(tableView, indexPath.row))
			return [theCell.Orphan() autorelease];
		}
	return null;
	}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<CGFloat> theQ = theSection->RowHeight(indexPath.row))
			return theQ.Get();
		}
	return tableView.rowHeight;
	}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:section])
		{
		if (ZQ<CGFloat> theQ = theSection->HeaderHeight())
			return theQ.Get();
		}
	return tableView.sectionHeaderHeight;
	}

- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:section])
		{
		if (ZQ<CGFloat> theQ = theSection->FooterHeight())
			return theQ.Get();
		}
	return tableView.sectionFooterHeight;
	}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:section])
		{
		if (ZQ<string8> theQ = theSection->HeaderTitle())
			return ZUtil_NSObject::sString(theQ.Get());
		}
	return null;
	}

- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:section])
		{
		if (ZQ<string8> theQ = theSection->FooterTitle())
			return ZUtil_NSObject::sString(theQ.Get());
		}
	return null;
	}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:section])
		{
		if (ZRef<UIView> theView = theSection->HeaderView())
			return [theView.Orphan() autorelease];
		}
	return null;
	}

- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:section])
		{
		if (ZRef<UIView> theView = theSection->FooterView())
			return [theView.Orphan() autorelease];
		}
	return null;
	}

- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:indexPath.section])
		theSection->AccessoryButtonTapped(indexPath.row);
	}

- (BOOL)tableView:(UITableView *)tableView shouldIndentWhileEditingRowAtIndexPath:(NSIndexPath *)indexPath
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<bool> theQ = theSection->ShouldIndentWhileEditing(indexPath.row))
			return theQ.Get();
		}
	return true;
	}

- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath
	{
	if (ZRef<UITVC_Section> theSection = [self pGetSection:indexPath.section])
		{
		if (ZQ<UITableViewCellEditingStyle> theQ = theSection->EditingStyle(indexPath.row))
			return theQ.Get();
		}
	return UITableViewCellEditingStyleNone;
	}

- (void) addSection:(ZRef<UITVC_Section>) iSection
	{
	fSections.push_back(iSection);
	}

-(ZRef<UITVC_Section>)pGetSection:(size_t)iIndex
	{
	if (iIndex < fSections.size())
		return fSections[iIndex];
	return null;
	}

@end // ZooLib_UITVC_WithSections
