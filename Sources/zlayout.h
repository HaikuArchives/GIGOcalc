#ifndef ZLAYOUT_H
#define ZLAYOUT_H

#include <MessageFilter.h>
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <MessageRunner.h>
#include <Messenger.h>

#include <OS.h>

#include <stdlib.h>
#include <typeinfo>

/*********************************************************************
	Shamyl Emrich Zakariya
	zakariya@earthlink.net
	August 2000
	
	ZLayout!
	A simple but friendly layout management system for BeOS.
	The idea behind this layout manager is two-fold. First, as we all know, the Be API
	is critically lacking in layout management. I've tried a couple different home-brew APIs
	from BeBits, and found them functionally superb but alien & complex in implementation -- which
	is to say they require a huge .so file and have proprietary APIs which are alien to the
	mood and 'feel' of the BeAPI, which I think is superbly written and devised; and hence a joy
	to program under.
	
	ZLayout attempts to address this issue by providing a _very_ simple framework which
	works almost entirely within the hook functions provided by the Be API. No need to use some
	strange subclass of BWindow or BApplication with it's own hooks and curiousities. All you do
	is replace your BViews with ZLayoutViews and pass in a layout manager in the constructor.
	That's it! You use normal AddChild() and AddChildAt() calls (and one extra, AddChildWithConstraint()),
	and the rest is taken care of for you.
	
	I've provided an abstract layout manager superclass and a few subclasses, ZBorderLayout, ZGridLayout,
	ZFlowLayout, ZHorizontalLayout, ZVerticalLayout. I've also provided a split pane implementation, ZSplitPane
	which allows for a user-positionable divider between two views. The layout managers are all fairly straightforward,
	the first three are atken from java, and the last two are present in every layout API I've ever run across. The
	idea, really, is simplicity, and the removal of ugly details form layout; take, for example, the insertion of a menu
	bar into a window. In the old days you'd make the menu, let it size itself when it's added, and use it's frame() to 
	position the rest of the contents. Now, with ZBorderLayout, you'd just call AddChildAt(menubar, ZBorderLayout::NORTH)
	and then put your stuff in ZBorderLayout::SOUTH. Everything's peachy! Font-sensitive, too.
	
	One obvious note: My ZLayoutViews will happily accept ZLayoutViews as children. Nesting different layout schema
	is the whole point here -- since it expediates complex layout. It's not required, however, that children are ZLayoutViews;
	just keep in mind, that while ZLayoutViews return meaningful data when GetPreferredSize() is called most BeOS gui objects
	do not -- they just return their current proportions. My layout managers care about their contents, and
	want them to be happy, and the top level ZLayoutView will resize the window automagically to layout contents properly.
	Thus, you may want to subclass objects like BTextView to return meaningful data when GetPreferredSize is called.
	
	There are a couple more notes, however, that should be kept in mind. Since the ZLayoutView attempts
	to manage PreferredSize()s of it's children, the topmost view (the one which constitues the entire
	bounds of the parent Window()) must be instantiated with the window's Bounds() as it's frame rectangle.
	This is because the topmost view will intercept B_ZOOM messages and insure the window, when the zoom
	button is clicked, will resize to happily display it's contents at their preferred sizes (though it won't let itself
	become larger than the screen rectangle, nor will any edges go offscreen). Thus, while my API doesn't require
	you to use only ZLayoutViews, it does require the topmost view to be a ZLayoutView, taking up the
	entire frame of the window if you want the Zoom button on the window tab to resize the window to layout its
	contents happily; thusly, if you pass it another size than the window's Bounds() it won't know it's
	top-level and things will act funny. Also, you may find the contents will layout oddly (eg, the top 
	ZLayoutView, being another size than the window, will layout its content to that size, and not the window's size. This
	isn't by any means a requirement. If you just want to use a ZLayoutView to layout a particularly ornery control panel
	and use Be's normal layout APIs to do the rest, feel free. Just be sure to implement BWindow::Zoom() if you want that
	functionality. You can trust, at least, that the ZLayoutViews you do use will return valid data when you call GetPreferredSize
	on them.
	
	Also, I've provided for convenience's sake a stub rectangle, Z_NULLRECT which you can use in the constructor
	of any item which is being layed out by a ZLayoutView. Since the layout manager will handle position and size, the
	rectangle passed in to the object's constructor is moot, anyway. It's convenient, nothing more.

*********************************************************************/

//#define DEBUG_BASE 666
//#define DEBUG_LV 666
//#define DEBUG_BORDER 666
//#define DEBUG_GRID 666
//#define DEBUG_STRETCH 666
//#define DEBUG_FLOW 666

#define Z_NULLRECT BRect(0,0,100,100)

//just a prototype
class ZLayoutView;

/**************************************
	class ZConstraint
	Some layout managers will need more
	complex info for layout management
	than a simple int.

	ZConstraint is an abstract base class
	which can be extended for use with
	certain managers. As of Jan 25 2000
	the only manager using ZConstraint
	is ZStretchLayout, which subclasses
	ZStretchConstraint.
**************************************/

class ZConstraint{
	public:
		ZConstraint();
};


/**************************************
	class ZLayoutManager
	An abstract class intended to be
	subclassed into a useful layout
	manager.
	Subclasses must at least implement
	virtual void Layout(float, float)
	but can subclass other methods for
	more custom implementations

**************************************/


class ZLayoutManager{
	protected:
		bool _doDraw;
		int32 _size;

		
		BView ** _viewList;
		
	public:
		ZLayoutManager(int32 size);
		virtual ~ZLayoutManager();
		
		virtual void AddChild(BView *v);
		virtual void AddChildAt(BView *v, int32 at = 0);
		virtual void AddChildWithConstraint(BView *v, ZConstraint *constraint);
		virtual void RemoveChild(BView *v);
		virtual void RemoveChild(int32 i);
		
		BView *ChildAt(int32 i);
		int32 IndexOfChild(BView *v);
		int32 CountChildren();
		int32 Size();

		//THIS *MUST* BE IMPLEMENTED BY SUBCLASSES
		virtual void Layout(float w, float h) = 0;
		
		//not as important to be implemented by children
		//will be called by ZLayoutView::DrawAfterChilren
		//nice, if you want your layout manager to draw some
		//sort of cruft, like border lines or somehting
		virtual void Draw(ZLayoutView *v, float w, float h){}

		//if you want your layout manager to be content-smart
		virtual void GetPreferredSize(float *width, float *height, float currentWidth, float currentHeight);

		void SetDrawing(bool tf);
		bool ShouldDraw();
};


/**************************************
	class ZLayoutView
	A simple subclass of BView. 
	Must be given in constructor a
	subclass of ZLayoutManager, which it
	will take posession of and delete
	when it itself is deleted.

	Functionality is more or less
	identical to BView, except for the 
	addition of one function
	void AddChildWithConstraint(BView *, ZConstraint *)
	 which is used right now only by ZStretchLayout
		
**************************************/
class ZWindowZoomer;


class ZLayoutView : public BView{
	protected:
		float _lastWidth, _lastHeight;
		float _preferredWidth, _preferredHeight;
		bool _topmost;
		ZWindowZoomer *_windowZoomer;
		
		ZLayoutManager *_layoutManager;

		void setFlags(BView *v);
		
	public:
		ZLayoutView(BRect frame, const char *name, ZLayoutManager *lm, uint32 resize = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = 0);
		virtual ~ZLayoutView();
				
		void AddChild(BView *v, BView *sibling = NULL);
		void AddChildAt(BView *v, int32 where = 0);
		void AddChildWithConstraint(BView *v, ZConstraint *constraint);
		
		bool RemoveChild(BView *v);

		virtual void AttachedToWindow();
		virtual void AllAttached();
		virtual void GetPreferredSize(float *width, float *height);
		virtual void ResizeToPreferred();
		virtual void FrameResized(float width, float height);
		virtual void DrawAfterChildren(BRect r);
		
		ZLayoutManager * LayoutManager();
		
		void InvalidateLayout();
		
		void SetTopmost(bool tf);
		bool IsTopmost();
		void SetPreferredSize(float width, float height);
		void DrawLayout(bool tf);
};


//**************************************************

class ZWindowZoomer : public BMessageFilter{
	protected:
		ZLayoutView *_topmostView;

	public:
		ZWindowZoomer(ZLayoutView *);
		~ZWindowZoomer();
		
		virtual filter_result Filter(BMessage *msg, BHandler **target);
};


/********************************************
	ZBorderLayout
	Operates very similarly to the java BorderLayout
	class.
	In a nutshell, when you add a child to a ZBorderLayout
	managed view, you give it one of the public static members
	SOUTH, NORTH, EAST, WEST, CENTER.
	
	...and it will stuff that child in. If there's already a child there,
	it won't do anything. The Associatiion of a view with 
	a corner is a tougher issue. If the first view added is north, that view
	will get the north-west and north-east corners. If the next view added is
	east, it will get the south-east corner but not north-east. First come first serve.
	When views are removed, corners are re-associated to the remaining
	children.

********************************************/

class ZBorderLayout : public ZLayoutManager{
	public:
		const static long EAST = 0;
		const static long WEST = 1;
		const static long NORTH = 2;
		const static long SOUTH = 3;
		const static long CENTER = 4;

	protected:

		float _nMargin, _eMargin, _sMargin, _wMargin;
		int32 _ne, _nw, _sw, _se;
		
		void _associateCorners(BView *v);
		void _disAssociateCorners(BView *v);
		
	public:
		ZBorderLayout(int margin = 0);
		ZBorderLayout(int nM, int sM, int eM, int wM);
		virtual ~ZBorderLayout();

		virtual void AddChild(BView *v);
		virtual void AddChildAt(BView *v, int32 at = 0);
		virtual void RemoveChild(BView *v);
		virtual void RemoveChild(int32 i);
		
		void SetMargins(int n = 0, int s = 0, int e = 0, int w = 0);
		void Margins(int *n, int *s, int *e, int *w);
		void Layout(float w, float h);

		virtual void Draw(ZLayoutView *v, float w, float h);
		virtual void GetPreferredSize(float *width, float *height, float currentWidth, float currentHeight);
};

/*************************************************
	ZGridLayout
	Like the classic java grid layout, but with a couple nice details.
	Instantiate with a fixed number of rows, and columns (not changeable after
	instantiation). If you call SetCellHeight/Width this will set the max-size
	which a cell can take. That is to say, for example, if diagonally resizing a
	view which has the cell heights but not widths set, the contents will
	max out vertically at the pixel height you set it to, but the width will
	continue to stretch.

*************************************************/

class ZGridLayout : public ZLayoutManager{
	protected:
		int32 _cols, _rows;
		float _hPad, _vPad;
		float _cellHeight, _cellWidth; 
		
	public:
		ZGridLayout(int32 cols, int32 rows, int hPad= 2, int vPad = 2);
		~ZGridLayout();
		
		virtual void Draw(ZLayoutView *v, float w, float h);
		virtual void GetPreferredSize(float *width, float *height, float currentWidth, float currentHeight);

		void Layout(float w, float h);
		void SetCellHeight(float h);
		void SetCellWidth(float w);
		float CellHeight();
		float CellWidth();
};

/*********************************************************
	ZFlowLayout
	Works much like the layout manager of same name in java.
	Simply allows for a 'flow' of BViews from right to left or left to right.
	From a UI standpoint, this makes most sense when you want to have a
	row of buttons or some fairly homegenous group of items. The one
	real caveat is that this layout manager, while sensitive to the preferred
	widths of it's children, will force a common height. This only makes sense
	when you think about how a flow layout should be rendered. Eg, you wouldn't
	want differing heights anyway, because it would throw off the baseline. Therefore,
	this is best with items which agreeably will accept the same height.
	For example, a row of buttons, icons, or BMenuFields... they'd all work well
	together with a forced height as long as that forced height is reasonable.

	Another note, I added justification to the layout such that all rows
	but the last will have a common width. This is nice looking, but a little
	heavy on redrawing. Iw works by laying out the rows based on preferred
	widths, and than going back and stretching each item enough to fill out the
	row. It won't, however, compress items to be thinner than their respective
	preferred widths. This functionality is off by default, but calling SetJustified(bool)
	will turn it on or off. This is nice for buttons, or stretchable things, but I imagine
	it would look funny on some sort of fixed-width icon, like for example the
	items in the toolbar for MS Word. They want to be square, and are designed to
	be so, so justification would distort them (by making them wider). Also, justification
	is not performed on the last row. Since, after all, the last row might have only
	a couple items on it and stretching them would 'look funny'. If you don't like this,
	feel free to hack my code... I don't mind. Just tell me, eh?

********************************************************/
class ZFlowLayout : public ZLayoutManager{
	public:
		const static long RIGHT = 0;
		const static long LEFT = 1;
		
	protected:
		long _align;
		bool _justify;
		
		float _hPad, _vPad;
		float _fixedHeight, _fixedWidth;
		float _preferredWidth, _preferredHeight;
		int *_rowList;
		
	public:
		ZFlowLayout(long alignment, float height=32, float width = 0, int hPad = 2, int vPad = 2, int children=36);
		~ZFlowLayout();
		
		virtual void Draw(ZLayoutView *v, float w, float h);
		virtual void Layout(float w, float h);
		virtual void GetPreferredSize(float *w, float *h, float currentWidth, float currentHeight);

		void SetFixedWidth(float w = 0);
		float FixedWidth();
		void SetFixedHeight(float h);
		float FixedHeight();
		
		void SetJustified(bool tf);
		bool Justified();
		
		void SetAlignment(long a);
		long Alignment();
};

/************************************************************
	ZHorizontalLayout, ZVerticalLayout

	Like a ZFlowLayout but more specialized. Doesn't provide multiple lines
	or wrapping. Rather it's assumed the contents will all be on one row or column.
	Why use this? Because if you _know_ you want your contents all on one
	row or column and not wrapping, this is a little slimmer, and faster. Also, this allows
	the setting of alignment within the row/column. Eg, you can add three buttons
	aligned left/top, and one aligned right/bottom. Very convenient.

************************************************************/

class ZHorizontalLayout: public ZLayoutManager{
		
	protected:
		int _spring;
		float _hPad, _vPad;
		float _fixedHeight, _fixedWidth;
		BRect *_childRects;
		bool _justify;
		
	public:
		ZHorizontalLayout(int hPad = 2, int vPad = 2, int children = 36);
		~ZHorizontalLayout();
		
		void PutSpring(int where);
		void RemoveSpring();
		
		void SetFixedWidth(float w = 0);
		float FixedWidth();
		void SetFixedHeight(float h);
		float FixedHeight();
		void SetJustified(bool j);
		bool Justified();
		
		virtual void Layout(float w, float h);
		virtual void GetPreferredSize(float *w, float *h, float currentWidth, float currentHeight);
};

class ZVerticalLayout: public ZLayoutManager{
	protected:
		int _spring;
		float _hPad, _vPad;
		float _fixedHeight, _fixedWidth;
		BRect *_childRects;
		bool _justify;
				
	public:
		ZVerticalLayout(int hPad = 2, int vPad = 2, int children = 36);
		~ZVerticalLayout();
		
		void PutSpring(int where);
		void RemoveSpring();
		
		void SetFixedWidth(float w = 0);
		float FixedWidth();
		void SetFixedHeight(float h);
		float FixedHeight();
		void SetJustified(bool j);
		bool Justified();

		virtual void Layout(float w, float h);
		virtual void GetPreferredSize(float *w, float *h, float currentWidth, float currentHeight);
};

/**********************************************************
	ZSplitPane
	A very straightforward but good splitpane container. Provides some nice functionality
	which run-of-the-mill splitters don't seem to have, for example:
	
		Settable following mode; When the container is resized, does the first or second
		child resize? Use SetFollowingMode with ZSplitPane::FOLLOW_FIRST or FOLLOW_SECOND
		to set this
		
		Snappable; which is to say, if you click once on the dragger it will snap shut the
		side which is set to be followed. So, for example, say you have a column on the left which
		is set via FOLLOW_FIRST to be fixed width on resize. When the user clicks on the dragger
		the left side is snapped shut, and the right side becomes dominant. When clicked again, the left
		side reopens to its last position.
		
		Nice cursors, which change accordingly to the status (horizontal, vertical, snapped)
		
		Orientation settable at runtime
		
		Easy to subclass your own style of dragger, by implementing the DrawSplitter(...) method
		
		implemented GetPreferredSize(...); so this is usable from within a ZLayoutView
		
	One caveat: To get the cursors, you must include the file zlayout_cursors.rsrc in your project
	since the cursors are stored as resources rather than being defined in a .h file. It's not so much because
	I'm lazy as because I just don't have the sort of mind that is capable of defining a bitmap via hexadecimal.
	
	the function _draw3DRect(...) is a convenience function for drawing a sunken, raised or flush rectangle; it's used
	to draw the 'grippers' in the dragger. It expects ZSplitPane::RAISED/SUNKEN/FLUSH for the parameter frameStyle
	the rgb_colors bg, light, and dark are the colors it needs to draw the raised and sunken effects
		bg: color of rectangle
		light: color of highlight
		dark: color of shadow

**********************************************************/
void _draw3DRect(BView *v, BRect r, rgb_color bg, rgb_color light, rgb_color dark, long frameStyle);

class ZSplitPane : public BView
{
	public:
		const static long HORIZONTAL = 0;
		const static long VERTICAL = 1;

		const static long FOLLOW_FIRST = 2;
		const static long FOLLOW_SECOND = 3;
		
		const static long SNAP_FIRST = 4;
		const static long SNAP_SECOND = 5;
		
		const static long RAISED = 6;
		const static long SUNKEN = 7;
		const static long FLUSH = 8;

	protected:
		long _orientation, _follow, _snappedAt;
		BView *_firstChild, *_secondChild;
		
		float _lastWidth, _lastHeight, _splitterWidth, _snapDistance;
		BRect _splitterRect, _preSnapSplitterRect;
		
		bool _mouseDown, _snapped;
		BPoint _mouseDownPosition;
		
		BMessageRunner *_messageRunner;
		
		//six cursors. _h cursors are horizontal, _v are vertical
		//'LR' means cursor show left and right arrow, 'L' means only left, 'R' means only right
		//'TB' means cursor shows top and bottom arrows. 'T' top only, 'B' bottom only.
		//cursor is set in MouseMoved() according to state of splitter and orientation
		//which is to say if the layout is vertical and the splitter is dragged (or snapped) to the
		//bottom, the cursor will change to _vCursorT -- showing only an upward arrow instead of
		//both upward and downward.
		BCursor *_hCursorLR, *_vCursorTB, *_hCursorL, *_hCursorR, *_vCursorT, *_vCursorB;

		//this guys actually does all the work of resizing and so on.
		void _doLayout();
		void _stopAnimation();
		
	public:
		ZSplitPane(BRect frame, const char *name, long orientation = HORIZONTAL, uint32 resize = B_FOLLOW_ALL_SIDES, uint32 flags = 0);
		virtual ~ZSplitPane();

		//add/remove the top or left child		
		void AddFirstChild(BView *child);
		void RemoveFirstChild();
		
		//add/remove bottom or right child
		void AddSecondChild(BView *child);
		void RemoveSecondChild();

		//set position for splitter, in screen coordinates
		//position of 0 or Bounds().right/bottom will snap the splitter		
		void SetSplitPosition(float where);
		float SplitPosition();
		
		//set which side will be constant size during resize of frame
		//expects ZSplitPane::FOLLOW_FIRST/FOLLOW_SECOND
		void SetFollowingMode(long follow);
		long FollowingMode();

		//set layout orientation, expects ZSplitPane::VERTICAL/HORIZONTAL
		void SetOrientation(long orient);
		long Orientation();

		//standard BView guts		
		virtual void AttachedToWindow();
		virtual void FrameResized(float width, float height);
		virtual void GetPreferredSize(float *width, float *height);
		virtual void MouseDown(BPoint loc);
		virtual void MouseMoved(BPoint loc, uint32 transit, const BMessage *msg);
		virtual void MouseUp(BPoint loc);		
		virtual void MessageReceived(BMessage *msg);

		virtual void Draw(BRect update);

		//implement this method to draw your custom splitter
		virtual void DrawSplitter(BRect splitter, long orientation, bool mouseDown);
		
		void Open();
		void Close();
		
		void InvalidateLayout(){_doLayout();}
};

/************************************************************
	class ZSeparator derived from BView
	Just a spacer, drawing a thin line horizontally or vertically

************************************************************/

class ZSeparator : public BView
{
	public:
		const static long HORIZONTAL = 0;
		const static long VERTICAL = 1;

	protected:
		float _padding;
		long _orient;
		
	public:
		ZSeparator(BRect frame, const char *name, long orientation, float padding = 0, uint32 follow = B_FOLLOW_NONE);
		~ZSeparator();
		
		virtual void Draw(BRect update);
		virtual void GetPreferredSize(float *w, float *h);
		virtual void AttachedToWindow();
};

#endif