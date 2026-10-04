#pragma once
/*
scitem2.h
BoltOS Usermode Include Declarations
The message-driven System Control page

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"
#include "wndproc2.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 __ScItemProcedure2: a page of System Control, told what has happened rather
 than asked to redraw itself. The same contract as __WindowProcedure2, with a
 panel where a window would be:

     WORD64 __ScItemProcedure2(HANDLE Panel, WORD32 Message,
                               WORD64 Param1, WORD64 Param2);

 The panel is made by the host and handed over at MSG_INIT. Everything the
 page puts on it is placed against the panel's own corner, so a page does not
 know or care where in the window it has been put, and the host can hide the
 whole page in one call.

 The host forwards every message it is given, after its own controls have had
 it, so a page is written the way an application is: build at MSG_INIT, read
 CmnCtl2_WasClicked on anything else, and free at MSG_QUIT.

 Three things are different from a window. Only the page on screen is sent
 MSG_PAINT, MSG_TIMER and input. MSG_PAINT carries the panel's rectangle in
 window coordinates, for the sake of a page that draws something of its own
 rather than putting every last word in a control. And a page lives in a
 module that may never have been started, because the scan that finds pages
 loads every application at boot and calls __modmain on none of them: a page
 allocates its own state at MSG_INIT and must not read anything its
 application set up.
 */

/* -- Messages -- */

/*
 The window messages in wndproc2.h are used unchanged, so that one table of
 numbers covers both and a page can hand what it is given straight to
 CmnCtl2_Dispatch. The three below are what a page has and a window does not.

 Numbered from 0x40, clear of the window messages rather than continuing them.
 SCMSG_SHOW was 0x11, which MSG_INTERVAL later took: the host forwards every
 message it is given, so a page holding a timer read every tick as a show.
 */

/* This page is now the one on screen. Sent to the page that starts on screen
   as soon as it has been built, and to each page as it is selected. A page
   that reads a preference somebody else can change re-reads it here. */
#define SCMSG_SHOW 0x40

/* Another page has been selected. The page keeps its controls and its state;
   nothing of it will be drawn until the next SCMSG_SHOW. */
#define SCMSG_HIDE 0x41

/*
 The panel is now the size in Param1, packed as MSG_MAKEPOINT. Sent to every
 page, not only the one on screen, because a page put away at one size and
 brought back at another would lay out for neither.
 */
#define SCMSG_RESIZE 0x42

/*
 The smallest panel this page can lay out in, returned packed as
 MSG_MAKEPOINT. Zero is no opinion, which is what a page that has never been
 asked returns, and the host reads it as no floor of its own.

 The host refuses a window narrower than the widest answer, so one page that
 cannot shrink holds the whole window at its width rather than being drawn
 with its right hand side cut off.
 */
#define SCMSG_MINSIZE 0x43

/* -- Registration -- */

/*
 A page is a MODRES_RSRCTYPE_SCITEM2 resource, carrying RSRCTYPE_SCITEM2.
 Procedure2 is what the current host calls. Procedure is what a host built
 before this existed calls, and it has to be something safe to call, since
 that host calls it without looking.

 A separate resource type rather than a longer version of the old one, because
 __modres returns a pointer and no length. A reader that knows only the old
 type skips this one; a reader that knows both never reads a field off the end
 of a descriptor that does not have it.
 */

#endif /* BOLTSDK_APIV_10 */
