#ifndef __BRAINCHILD_CONTROL__
#define __BRAINCHILD_CONTROL__
/*
 *	brainchild.h
 *
 *	(C) Copyright 1993-2006 Jan van den Baard.
 *	    All Rights Reserved.
 *
 *	Brainchild custom editor control definitions.
 */

#include <windows.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 *	Class constants.
 */
#define BRAINCHILD_CLASS	_T( "BrainchildEditControl32" )

/*
 *	Caret styles.
 */
#define CARET_NORMAL		0
#define CARET_WIDE		1
#define CARET_BLOCK		2
#define CARET_HORIZONTAL	3

/*
 *	Notification messages.
 */
#define NMBC_CARETPOSITION		( NM_FIRST + 1 ) /* Uses the NMCARETPOSITION structure. */
#define NMBC_STATUSUPDATE		( NM_FIRST + 2 ) /* Uses the NMSTATUSUPDATE structure. */
#define NMBC_FILENAMECHANGED		( NM_FIRST + 3 ) /* Uses the NMFILENAMECHANGED structure. */
#define NMBC_MARKERSTATECHANGING	( NM_FIRST + 4 ) /* Uses the NMMARKERSTATE structure. */
#define NMBC_MARKERSTATECHANGED		( NM_FIRST + 5 ) /* Uses the NMMARKERSTATE structure. */
#define NMBC_MARGINCLICK		( NM_FIRST + 6 ) /* Uses the NMMARGINCLICK structure. */
#define NMBC_FINDCHANGED		( NM_FIRST + 7 ) /* Uses the NMFINDCHANGED structure. */

/*
 *	Sent to the control it's parent window
 *	whenever the caret position changes.
 */
typedef struct tagNMCARETPOSITION
{
	NMHDR			hdr;
	POINT			ptCaretPos;
} NMCARETPOSITION, FAR* LPNMCARETPOSITION;

/*
 *	Sent to the control it's parent window
 *	whenever the status of one of the described
 *	flags changes.
 */
typedef struct tagNMSTATUSUPDATE
{
	NMHDR			hdr;
	/*
	 *	Use these to determine wether a corresponding
	 *	menu item and/or toolbar button must be enabled
	 *	or disabled.
	 */
	BOOL			bAnyText;
	BOOL			bCanUndo;
	BOOL			bCanRedo;
	BOOL			bCanCut;
	BOOL			bCanCopy;
	BOOL			bCanPaste;
	BOOL			bCanDelete;
	BOOL			bModified;
	BOOL			bOverwrite;
	BOOL			bReadOnly;
	BOOL			bBookmarks;
	/*
	 *	File mode. (See below)
	 */
	int			nFileMode;
} NMSTATUSUPDATE, FAR* LPNMSTATUSUPDATE;

/*
 *	Supported file modes.
 */
#define FMODE_MSDOS		0
#define FMODE_UNIX		1
#define FMODE_MACINTOSH		2

/*
 *	Sent to the control it's parent window
 *	whenever the file name is changed.
 */
typedef struct tagNMFILENAMECHANGED
{
	NMHDR			hdr;
	LPCTSTR			lpszFileName;
	LPCTSTR			lpszFileTypeDesc;
	HICON			hFileTypeIconSmall;
	HICON			hFileTypeIconLarge;
} NMFILENAMECHANGED, FAR *LPNMFILENAMECHANGED;

/*
 *	Sent to the control it's parent window
 *	whenever somebody tries to change or has
 *	changed the state of a line marker.
 */
typedef struct tagNMMARKERSTATE
{
	NMHDR			hdr;
	int			nMarker;
	int			nLine;
	BOOL			bState;
	BOOL			bAllowState;
} NMMARKERSTATE, FAR* LPNMMARKERSTATE;

/*
 *	Sent to the control it's parent window
 *	when somebody clicks a mouse button inside
 *	the control margin.
 */
typedef struct tagNMMARGINCLICK
{
	NMHDR			hdr;
	int			nLine;
	int			nMargin;
	int			nButton;
	DWORD			dwButtonState;
	BOOL			bDoDefault;
} NMMARGINCLICK, FAR* LPNMMARGINCLICK;

/*
 *	Margin types.
 */
#define MCT_SELECTION		1
#define MCT_LINENUMBERS		2

/*
 *	Sent to the control it's parent window
 *	when on of the find/replace parameters
 *	changed.
 */
typedef struct tagNMFINDCHANGED
{
	NMHDR			hdr;
	LPCTSTR			lpszFindString;
	LPCTSTR			lpszReplaceString;
	DWORD			dwFlags;
} NMFINDCHANGED, FAR* LPNMFINDCHANGED;

#define FCF_CASE_SENSITIVE	( 1 << 0 )
#define FCF_WHOLE_WORDS_ONLY	( 1 << 1 )
#define FCF_INCREMENTAL_MODE	( 1 << 2 )

/*
 *	BCM_LOADFILE/BCM_SAVEFILE return codes.
 */
#define BCMR_OK				1	/* No problems. */
#define BCMR_ERROR			2	/* Error, Use GetLastError(). */
#define BCMR_SAVEFIRST			3	/* File modified, must save first. */
#define BCMR_BACKUP_ERROR		4	/* File backup failed. */

/*
 *	Caret commands.
 */
#define CID_CARET_RIGHT			1
#define CID_CARET_RIGHTEXTEND		2
#define CID_CARET_LEFT			3
#define CID_CARET_LEFTEXTEND		4
#define CID_CARET_UP			5
#define CID_CARET_UPEXTEND		6
#define CID_CARET_DOWN			7
#define CID_CARET_DOWNEXTEND		8
#define CID_CARET_EOL			9
#define CID_CARET_EOLEXTEND		10
#define CID_CARET_SOL			11
#define CID_CARET_SOLEXTEND		12
#define CID_CARET_NEXTWORD		13
#define CID_CARET_NEXTWORDEXTEND	14
#define CID_CARET_PREVWORD		15
#define CID_CARET_PREVWORDEXTEND	16
#define CID_CARET_HOME			17
#define CID_CARET_HOMEEXTEND		18
#define CID_CARET_END			19
#define CID_CARET_ENDEXTEND		20
#define CID_CARET_TABBACK		21
#define CID_CARET_TABBACKEXTEND		22
#define CID_CARET_VIEWUP		23
#define CID_CARET_VIEWUPEXTEND		24
#define CID_CARET_VIEWDOWN		25
#define CID_CARET_VIEWDOWNEXTEND	26
/*
 *	Miscellanious commands.
 */
#define CID_MISC_CLEARUNDOBUFFERS	50
/*
 *	View commands.
 */
#define CID_SCROLLVIEW_UP		80
#define CID_SCROLLVIEW_DOWN		81
#define CID_SCROLLVIEW_LEFT		82
#define CID_SCROLLVIEW_RIGHT		83
/*
 *	Edit commands.
 */
#define CID_EDIT_UNDO			100
#define CID_EDIT_REDO			101
#define CID_EDIT_PULL			102
#define CID_EDIT_DELETEEOL		103
#define CID_EDIT_BACKSPACE		104
#define CID_EDIT_DELETECHAR		105
#define CID_EDIT_DELETELINE		106
#define CID_EDIT_SPLITLINE		107
#define CID_EDIT_WORDTOUPPER		108
#define CID_EDIT_WORDTOLOWER		109
#define CID_EDIT_WORDSWAPCASE		110
#define CID_EDIT_COPYLINE		111
#define CID_EDIT_DELETEWORD		112
#define CID_EDIT_DELETEEOW		113
#define CID_EDIT_DELETESOW		114
#define CID_EDIT_TAB			115
#define CID_EDIT_BACKTAB		116
#define CID_EDIT_SWAPLINES		117
#define CID_EDIT_UPPERCASESELECTION	118
#define CID_EDIT_LOWERCASESELECTION	119
#define CID_EDIT_SWAPCASESELECTION	120
#define CID_EDIT_INSERTDATE		121
#define CID_EDIT_INSERTTIME		122
#define CID_EDIT_TOGGLEOVERWRITE	123
#define CID_EDIT_UNDOALL		124
#define CID_EDIT_REDOALL		125
#define CID_EDIT_SORTLINES		126
#define CID_EDIT_COMMENTSELECTION	127
#define CID_EDIT_UNCOMMENTSELECTION	128
/*
 *	Marker commands.
 */
#define	CID_MARK_WORD			200
#define CID_MARK_ALL			201
#define CID_MARK_CLEAR			202
#define CID_MARK_LINE			203
/*
 *	Clipboard commands.
 */
#define CID_CLIP_PASTE			225
#define CID_CLIP_COPY			226
#define CID_CLIP_CUT			227
#define CID_CLIP_DELETE			228
#define CID_CLIP_COPYLINE		229
#define CID_CLIP_CUTLINE		230
#define CID_CLIP_CLEARCLIPBOARD		231
#define CID_CLIP_COPYAPPEND		232
#define CID_CLIP_CUTAPPEND		233
/*
 *	Bookmark commands.
 */
#define CID_BOOKMARK_SET		250
#define CID_BOOKMARK_CLEAR		251
#define CID_BOOKMARK_TOGGLE		252
#define CID_BOOKMARK_CLEARALL		253
#define CID_BOOKMARK_GOTONEXT		254
#define CID_BOOKMARK_GOTOPREV		255
#define CID_BOOKMARK_GOTOFIRST		256
#define CID_BOOKMARK_GOTOLAST		257
/*
 *	Search commands.
 */
#define CID_FIND_NEXTWORD		275
#define CID_FIND_PREVWORD		276
#define CID_FIND_NEXT			277
#define CID_FIND_PREV			278
#define CID_FIND_MATCHBRACKET		279
#define CID_FIND_MATCHBRACKET_SELECT	280
#define CID_FIND_SET_INCREMENTAL_MODE	281
#define CID_FIND_CLEAR_INCREMENTAL_MODE	282
/*
 *	Dialog commands.
 */
#define CID_DIALOG_GOTO			300
#define CID_DIALOG_FIND			301
#define CID_DIALOG_REPLACE		302
#define CID_DIALOG_PROPERTIES		303
#define CID_DIALOG_ABOUTCONTROL		304
#define CID_DIALOG_PAGESETUP		305
/*
 *	File commands.
 */
#define CID_FILE_SAVE			401
#define CID_FILE_DELETECONTENTS		402
#define CID_FILE_PRINT			403

/*
 *	Command lists.
 */
typedef LPVOID			LPCOMLIST;

/*
 *	Command types.
 */
#define CTYPE_HARDCODED		0
#define CTYPE_TEXT		1
#define CTYPE_RUN		2
#define CTYPE_SHELLOPEN		3
#define CTYPE_END		0xFFFFFFFF

/*
 *	Command list routine prototypes.
 */
extern LPCOMLIST CreateCList( DWORD, ... );
extern void DeleteCList( LPCOMLIST );
extern DWORD NextCEntry( LPCOMLIST, LPDWORD, LPDWORD, LPDWORD );

/*
 *	Settings dialog prototypes.
 */
extern BOOL OpenSettingsDialog( HWND );

/*
 *	Miscellanious...
 */
extern BOOL LoadParserFromFile( LPCTSTR );
extern BOOL LoadParserFromMemory( HGLOBAL );
extern BOOL IsNonModalDialogMessage( const LPMSG );
extern BOOL UnloadParser( LPCTSTR pszTitle );

/*
 *	Structure used by the BCM_GETFILEINFO
 *	message.
 */
typedef struct tagFileInfo
{
	DWORD			dwSize;
	LPCTSTR			lpszFileName;
	LPCTSTR			lpszFileTypeDesc;
	HICON			hFileTypeIconSmall;
	HICON			hFileTypeIconLarge;
} FILEINFO, FAR *LPFILEINFO;

/*
 *	BCM_GOTOMARKER constants.
 */
#define GMK_NEXT			0
#define GMK_PREVIOUS			1
#define GMK_FIRST			2
#define GMK_LAST			3

/*
 *	BCM_SETFINDFLAGS constants.
 */
#define BFF_MATCH_CASE			( 1 << 0 )
#define BFF_WHOLE_WORDS_ONLY		( 1 << 1 )

/*
 *	BCM_SETCOLOR constants.
 */
#define BCC_TEXT			0
#define BCC_BACKGROUND			1
#define BCC_BACKGROUND_READONLY		2
#define BCC_SELECTION_MARGIN		3
#define BCC_BOOKMARK			4
#define BCC_NUMBER			5
#define BCC_DELIMITER			6
#define BCC_SELECTED_TEXT		7
#define BCC_SELECTED_BKGND		8
#define BCC_LINE_NUMBERS		9
#define BCC_BACKGROUND_LINE_NUMBERS	10
#define BCC_BACKGROUND_NUMBER		11
#define BCC_BACKGROUND_DELIMITER	12
#define BCC_BRACKET_MATCH		13
#define BCC_LINE_HIGHLIGHT		14
#define BCC_HYPERLINK			15
#define BCC_BACKGROUND_HYPERLINK	16
#define BCC_MARKER_1			17
#define BCC_MARKER_2			18
#define BCC_MARKER_3			19
#define BCC_MARKER_4			20
#define BCC_MARKER_5			21
#define BCC_MARKER_FG_1			22
#define BCC_MARKER_FG_2			23
#define BCC_MARKER_FG_3			24
#define BCC_MARKER_FG_4			25
#define BCC_MARKER_FG_5			26
#define BCC_COLUMN_OVERFLOW		27


/*
 *	Messages.
 */
#define BCM_BASE		( WM_USER + 0x0100 )
#define BCM_LOADFILE		( BCM_BASE + 1 )
#define BCM_SAVEFILE		( BCM_BASE + 2 )
#define BCM_SETLOGFONT		( BCM_BASE + 3 )
#define BCM_GOTOLINE		( BCM_BASE + 4 )
#define BCM_SETFILENAME		( BCM_BASE + 5 )
#define BCM_EXECUTECLIST	( BCM_BASE + 6 )
#define BCM_EXECUTECOMMAND	( BCM_BASE + 7 )
#define BCM_GETTEXT		( BCM_BASE + 8 )
#define BCM_GETTEXTLENGTH	( BCM_BASE + 9 )
#define BCM_GETWORD		( BCM_BASE + 10 )
#define BCM_GETWORDLENGTH	( BCM_BASE + 11 )
#define BCM_GETLINE		( BCM_BASE + 12 )
#define BCM_GETLINELENGTH	( BCM_BASE + 13 )
#define BCM_CHECKMODIFIED	( BCM_BASE + 14 )
#define BCM_SETTEXT		( BCM_BASE + 15 )
#define BCM_SYNTAXCOLORING	( BCM_BASE + 16 )
#define BCM_GETSELECTION	( BCM_BASE + 17 )
#define BCM_GETSELECTIONLENGTH	( BCM_BASE + 18 )
#define BCM_GETFILEINFO		( BCM_BASE + 19 )
#define BCM_SETREADONLY		( BCM_BASE + 20 )
#define BCM_SETCOLOR		( BCM_BASE + 21 )
#define BCM_GETCOLOR		( BCM_BASE + 22 )
#define BCM_INSERTLINE		( BCM_BASE + 23 )
#define BCM_LOADPARSER		( BCM_BASE + 24 )
#define BCM_SETUNICODEFLAG	( BCM_BASE + 25 )
#define BCM_GETUNICODEFLAG	( BCM_BASE + 26 )
#define BCM_REPLACESELECTION	( BCM_BASE + 27 )
#define BCM_GOTO		( BCM_BASE + 28 )
#define BCM_SETSELECTIONPOINTS	( BCM_BASE + 29 )
#define BCM_GETSELECTIONPOINTS	( BCM_BASE + 30 )
#define BCM_INSERTTEXT		( BCM_BASE + 31 )
#define BCM_NUMCHARSINLINE	( BCM_BASE + 32 )
#define BCM_SETMARKER		( BCM_BASE + 33 )
#define BCM_CLEARMARKER		( BCM_BASE + 34 )
#define BCM_TOGGLEMARKER	( BCM_BASE + 35 )
#define BCM_CLEARMARKERS	( BCM_BASE + 36 )
#define BCM_GOTOMARKER		( BCM_BASE + 37 )
#define BCM_SETFINDSTRING	( BCM_BASE + 38 )
#define BCM_SETREPLACESTRING	( BCM_BASE + 39 )
#define BCM_SETFINDFLAGS	( BCM_BASE + 40 )
#define BCM_SETPARSER		( BCM_BASE + 41 )

/*
 *	Status request messages.
 */
#define BCM_CANUNDO		( BCM_BASE + 51 )
#define BCM_CANREDO		( BCM_BASE + 52 )
#define BCM_CANCUT		( BCM_BASE + 53 )
#define BCM_CANCOPY		( BCM_BASE + 54 )
#define BCM_CANPASTE		( BCM_BASE + 55 )
#define BCM_CANDELETE		( BCM_BASE + 56 )
#define BCM_MODIFIED		( BCM_BASE + 57 )
#define BCM_OVERWRITE		( BCM_BASE + 58 )
#define BCM_READONLY		( BCM_BASE + 59 )
#define BCM_NUMBEROFLINES	( BCM_BASE + 60 )
#define BCM_ANYTEXT		( BCM_BASE + 61 )

/*
 *	Dialog messages.
 */
#define BCM_GOTO_DIALOG		( BCM_BASE + 100 )
#define BCM_FIND_DIALOG		( BCM_BASE + 101 )
#define BCM_REPLACE_DIALOG	( BCM_BASE + 102 )
#define BCM_PROPERTIES_DIALOG	( BCM_BASE + 103 )
#define BCM_SETTINGS_DIALOG	( BCM_BASE + 104 )
#define BCM_PAGESETUP_DIALOG	( BCM_BASE + 105 )

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __BRAINCHILD_CONTROL__ */