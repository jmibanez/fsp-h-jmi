/********************************************************
 ** F***ing Small Panel 0.7 Copyright (c) 2000-2001 By **
 ** Peter Zelezny <zed@linuxpower.org>                 **
 ** See file COPYING for license details.              **
 ********************************************************/

/********************************************************
My small hacked panel 1.0
Original code by Peter Zelezny
Changes by Adam D. Ruppe, 2007
Additional changes by JM Ibanez, 2008
*********************************************************/

#ifndef DO_ICON
#undef HAVE_XPM
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#ifdef HAVE_XPM
#include <X11/xpm.h>
#include "icon.xpm"
#endif

#include "fspanel.h"

/* you can edit these */
#define MAX_TASK_WIDTH 145
#define ICONWIDTH 16
#define ICONHEIGHT 16
#define WINHEIGHT 16//24
#define WINWIDTH (scr_width)
#define FONT_NAME "-*-lucida*-m*-r-*-*-12-*-*"

/* don't edit these */
#define TEXTPAD 6

Display *dd;
Window root_win;
Pixmap generic_icon;
Pixmap generic_mask;
GC fore_gc;
XFontStruct *xfs;
int scr_screen;
int scr_depth;
int scr_width;
int scr_height;
int text_y;
/*int time_width;*/
#if 0
unsigned short cols[] = {
	0xd75c, 0xd75c, 0xd75c,		  /* 0. light gray */
	0xbefb, 0xbaea, 0xbefb,		  /* 1. mid gray */
	0xaefb, 0xaaea, 0xaefb,		  /* 2. dark gray */
	0xefbe, 0xefbe, 0xefbe,		  /* 3. white */
	0x8617, 0x8207, 0x8617,		  /* 4. darkest gray */
	0x0000, 0x0000, 0x0000,		  /* 5. black */
	0x0000, 0x0000, 0xcccc		/* 6. blue */
};
#endif
#if 1
unsigned short cols[] = {
	0x3333, 0x3333, 0x3333,		  /* 0. */
	0x3333, 0x3333, 0x3333,		  /* 1. */
	0x3333, 0x3333, 0x3333,		  /* 2. */
	0x5555, 0x7878, 0xaaaa,		  /* 3. */
	0x8617, 0x8207, 0x8617,		  /* 4. */
	0xd9d9, 0xd9d9, 0xd9d9,		  /* 5. */
	0x0000, 0x0000, 0xcccc		  /* 6. */
};
#endif
/*
	Color meanings:

	0: background
	1: activated window background, clock background
	2: not used
	3: outline of buttons for 3d look, offset color for shaded windows
	4: minor outline of buttons, deactivated window text
	5: normal text
	6: urgent window

*/

#define PALETTE_COUNT (sizeof (cols) / sizeof (cols[0]) / 3)

unsigned long palette[PALETTE_COUNT];

char *atom_names[] = {
	"KWM_WIN_ICON",
	"_NET_CURRENT_DESKTOP",
	"_NET_CLIENT_LIST",
	"WM_STATE",
	"_NET_CLOSE_WINDOW",
	"_NET_ACTIVE_WINDOW",
	"_NET_WM_DESKTOP",
	"_NET_WM_WINDOW_TYPE",
	"_NET_WM_WINDOW_TYPE_DOCK",

	"_NET_WM_STATE",
	"_NET_WM_STATE_SHADED",
	"_NET_WM_STATE_SKIP_TASKBAR",
//	"_NET_WM_STATE_HIDDEN",
	"_NET_WM_STATE_DEMANDS_ATTENTION"
};

#define ATOM_COUNT (sizeof (atom_names) / sizeof (atom_names[0]))

Atom atoms[ATOM_COUNT];

#define atom_KWM_WIN_ICON atoms[0]
#define atom__NET_CURRENT_DESKTOP atoms[1]
#define atom__NET_CLIENT_LIST atoms[2]
#define atom_WM_STATE atoms[3]
#define atom__NET_CLOSE_WINDOW atoms[4]
#define atom__NET_ACTIVE_WINDOW atoms[5]
#define atom__NET_WM_DESKTOP atoms[6]
#define atom__NET_WM_WINDOW_TYPE atoms[7]
#define atom__NET_WM_WINDOW_TYPE_DOCK atoms[8]

#define atom__NET_WM_STATE atoms[9]
#define atom__NET_WM_STATE_SHADED atoms[10]
#define atom__NET_WM_STATE_SKIP_TASKBAR atoms[11]
//#define atom__NET_WM_STATE_HIDDEN atoms[12]
#define atom__NET_WM_STATE_DEMANDS_ATTENTION atoms[12] // something doesn't work with my implementation


/* Originally from wmctrl by Tomas Styblo */
static int client_msg(Window win, Atom msg,
        unsigned long data0, unsigned long data1, 
        unsigned long data2, unsigned long data3,
        unsigned long data4) {
    XEvent event;
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = msg;
    event.xclient.window = win;
    event.xclient.format = 32;
    event.xclient.data.l[0] = data0;
    event.xclient.data.l[1] = data1;
    event.xclient.data.l[2] = data2;
    event.xclient.data.l[3] = data3;
    event.xclient.data.l[4] = data4;
    
    if (XSendEvent(dd, root_win, False, mask, &event)) {
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}//////////////

void * get_prop_data (Window win, Atom prop, Atom type, int *items){
	Atom type_ret;
	int format_ret;
	unsigned long items_ret;
	unsigned long after_ret;
	unsigned char *prop_data;

	prop_data = 0;

	XGetWindowProperty (dd, win, prop, 0, 0x7fffffff, False,
							  type, &type_ret, &format_ret, &items_ret,
							  &after_ret, &prop_data);
	if (items)
		*items = items_ret;

	return prop_data;
}

void set_foreground (int index){
	XSetForeground (dd, fore_gc, palette[index]);
}

void draw_line (taskbar *tb, int x, int y, int a, int b){
	XDrawLine (dd, tb->win, fore_gc, x, y, a, b);
}

void fill_rect (taskbar *tb, int x, int y, int a, int b){
	XFillRectangle (dd, tb->win, fore_gc, x, y, a, b);
}

#ifdef DO_ICON
void scale_icon (task *tk){
	int x, y;
	unsigned int xx, yy, w, h, d, bw;
	Pixmap pix, mk = None;
	XGCValues gcv;
	GC mgc = 0;

	XGetGeometry (dd, tk->icon, &pix, &x, &y, &w, &h, &bw, &d);
	pix = XCreatePixmap (dd, tk->win, ICONWIDTH, ICONHEIGHT, scr_depth);

	if (tk->mask != None)
	{
		mk = XCreatePixmap (dd, tk->win, ICONWIDTH, ICONHEIGHT, 1);
		gcv.subwindow_mode = IncludeInferiors;
		gcv.graphics_exposures = False;
		mgc = XCreateGC (dd, mk, GCGraphicsExposures | GCSubwindowMode, &gcv);
	}

	set_foreground (3);


	/* this is my simple & dirty scaling routine */
	for (y = ICONHEIGHT - 1; y >= 0; y--)
	{
		yy = (y * h) / ICONHEIGHT;
		for (x = ICONWIDTH - 1; x >= 0; x--)
		{
			xx = (x * w) / ICONWIDTH;
			if (d != scr_depth)
				XCopyPlane (dd, tk->icon, pix, fore_gc, xx, yy, 1, 1, x, y, 1);
			else
				XCopyArea (dd, tk->icon, pix, fore_gc, xx, yy, 1, 1, x, y);
			if (mk != None)
				XCopyArea (dd, tk->mask, mk, mgc, xx, yy, 1, 1, x, y);
		}
	}

	if (mk != None)
	{
		XFreeGC (dd, mgc);
		tk->mask = mk;
	}

	tk->icon = pix;
}

void get_task_hinticon (task *tk){
	XWMHints *hin;

	tk->icon = None;
	tk->mask = None;

	hin = XGetWMHints (dd, tk->win);

	if (hin)
	{
		if ((hin->flags & IconPixmapHint))
		{
			if ((hin->flags & IconMaskHint))
			{
				tk->mask = hin->icon_mask;
			}

			tk->icon = hin->icon_pixmap;
			tk->icon_copied = 1;
			scale_icon (tk);
		}
		XFree (hin);
	}

	if (tk->icon == None)
	{
		tk->icon = generic_icon;
		tk->mask = generic_mask;
	}
}

void get_task_kdeicon (task *tk){
	unsigned long *data;

	data = get_prop_data (tk->win, atom_KWM_WIN_ICON, atom_KWM_WIN_ICON, 0);
	if (data)
	{
		tk->icon = data[0];
		tk->mask = data[1];
		XFree (data);
	}
}

#endif

unsigned long find_desktop (Window win){
	unsigned long desk = 0;
	unsigned long *data;

//	data = get_prop_data (win, atom__WIN_WORKSPACE, XA_CARDINAL, 0);
	if(win == root_win)
		data = get_prop_data (win, atom__NET_CURRENT_DESKTOP, XA_CARDINAL, 0);
	else
		data = get_prop_data (win, atom__NET_WM_DESKTOP, XA_CARDINAL, 0);
	if (data)
	{
		desk = *data;
		XFree (data);
	}
	return desk;
}

int is_iconified (Window win){
	unsigned long *data;
	int ret = 0;

	data = get_prop_data (win, atom_WM_STATE, atom_WM_STATE, 0);
	if (data)
	{
		if (data[0] == IconicState)
			ret = 1;
		XFree (data);
	}

	return ret;
}

#define STATE_ICONIFIED 	1
#define STATE_SHADED		2
#define STATE_HIDDEN		4
#define STATE_DEMANDS_ATTENTION 8

unsigned long get_state(Window win){
	Atom * data;
	int items,a;
	unsigned long state = 0;
	data = get_prop_data (win, atom__NET_WM_STATE, XA_ATOM, &items);
	if (data)
	{
		for(a=0; a < items; a++){
			if(data[a] == atom__NET_WM_STATE_SHADED)
				state |= STATE_SHADED;
			if(data[a] == atom__NET_WM_STATE_SKIP_TASKBAR)
				state |= STATE_HIDDEN;
			if(data[a] == atom__NET_WM_STATE_DEMANDS_ATTENTION)
				state |= STATE_DEMANDS_ATTENTION;
		}
		XFree (data);
	}

	if(is_iconified(win))
		state |= STATE_ICONIFIED;

	return state;

}

void add_task (taskbar * tb, Window win, int focus){
	task *tk, *list;
	unsigned long desk;
	unsigned long state;

	if (win == tb->win) return; /* Don't display the taskbar on the taskbar */

	/* is this window on a different desktop? */

	desk = find_desktop(win);

	/* Skip anything on a different desktop that is also NOT on all desktops */
	if (desk != 0xffffffff && tb->my_desktop != desk)
		return;

	state = get_state(win);
	if (state & STATE_HIDDEN)
		return;
	if((state & STATE_ICONIFIED) && !(state & STATE_SHADED))
		return;

	tk = calloc (1, sizeof (task));
	tk->win = win;
	tk->focused = focus;
	tk->name = get_prop_data (win, XA_WM_NAME, XA_STRING, 0);
	tk->iconified = state & STATE_ICONIFIED;

	tk->demands_attention = !!(state & STATE_DEMANDS_ATTENTION);

#ifdef DO_ICON
	get_task_kdeicon (tk);
	if (tk->icon == None)
		get_task_hinticon (tk);
#endif

	XSelectInput (dd, win, PropertyChangeMask | FocusChangeMask |
					  StructureNotifyMask);

	/* now append it to our linked list */
	tb->num_tasks++;

	list = tb->task_list;
	if (!list)
	{
		tb->task_list = tk;
		return;
	}
	while (1)
	{
		if (!list->next)
		{
			list->next = tk;
			return;
		}
		list = list->next;
	}
}

void gui_sync (void){
	XSync (dd, False);
}

void set_prop (Window win, Atom at, long val){
	XChangeProperty (dd, win, at, XA_CARDINAL, 32,
						  PropModeReplace, (unsigned char *) &val, 1);
}

taskbar * gui_create_taskbar (void){
	taskbar *tb;
	Window win;
	Atom type_atoms[1];

	XSizeHints size_hints;
	XSetWindowAttributes att;

	long val;

	att.background_pixel = palette[0];
	att.event_mask = ButtonPressMask | ExposureMask;

	win = XCreateWindow (
				  /* display */ dd,
				  /* parent  */ root_win,
				  /* x       */ 0,
				  /* y       */ scr_height - WINHEIGHT,
				  /* width   */ WINWIDTH,
				  /* height  */ WINHEIGHT,
				  /* border  */ 0,
				  /* depth   */ CopyFromParent,
				  /* class   */ InputOutput,
				  /* visual  */ CopyFromParent,
				  /*value mask*/ CWBackPixel | CWEventMask,
				  /* attribs */ &att);

	type_atoms[0] = atom__NET_WM_WINDOW_TYPE_DOCK;
	XChangeProperty(dd, win, atom__NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace,
							(unsigned char*) type_atoms, 1);

	/* make sure the WM obeys our window position */
	size_hints.flags = PPosition;
	/*XSetWMNormalHints (dd, win, &size_hints);*/
        val = 0xFFFFFFFF;
	XChangeProperty (dd, win, atom__NET_WM_DESKTOP,
							XA_CARDINAL, 32, PropModeReplace,
							(unsigned char *) &val, 1);
	XChangeProperty (dd, win, XA_WM_NORMAL_HINTS,
							XA_WM_SIZE_HINTS, 32, PropModeReplace,
							(unsigned char *) &size_hints, sizeof (XSizeHints) / 4);

	XMapWindow (dd, win);

	tb = calloc (1, sizeof (taskbar));
	tb->win = win;

	return tb;
}

void gui_init (void){
	XGCValues gcv;
	XColor xcl;
	int i, j;
	char *fontname;

	i = j = 0;
	do
	{
		xcl.red = cols[i];
		i++;
		xcl.green = cols[i];
		i++;
		xcl.blue = cols[i];
		i++;
		XAllocColor (dd, DefaultColormap (dd, scr_screen), &xcl);
		palette[j] = xcl.pixel;
		j++;
	}
	while (j < PALETTE_COUNT);

	fontname = FONT_NAME;
	do
	{
		xfs = XLoadQueryFont (dd, fontname);
		fontname = "fixed";
	}
	while (!xfs);

	/*time_width = XTextWidth (xfs, "88:88", 5); */
#define time_width (35)
	text_y = xfs->ascent + ((WINHEIGHT - xfs->ascent) / 2);

	gcv.font = xfs->fid;
	gcv.graphics_exposures = False;
	fore_gc = XCreateGC (dd, root_win, GCFont | GCGraphicsExposures, &gcv);

#ifdef HAVE_XPM
	XpmCreatePixmapFromData (dd, root_win, icon_xpm, &generic_icon,
									 &generic_mask, NULL);
#else
	generic_icon = 0;
#endif
}

void gui_draw_vline (taskbar * tb, int x){
	set_foreground (4);
	draw_line (tb, x, 0, x, WINHEIGHT);
	set_foreground (3);
	draw_line (tb, x + 1, 0, x + 1, WINHEIGHT);
}

void gui_draw_task (taskbar * tb, task * tk){
	int len;
	int x = tk->pos_x;
	int taskw = tk->width;

	if (!tk->name)
		return;

	gui_draw_vline (tb, x);

/*set_foreground (3); *//* it's already 3 from gui_draw_vline() */
	draw_line (tb, x + 1, 0, x + taskw, 0);

	set_foreground (1);
	draw_line (tb, x + 1, WINHEIGHT - 1, x + taskw, WINHEIGHT - 1);

	if (tk->focused)
	{
		x++;
		/*set_foreground (1);*/		  /* mid gray */

		fill_rect (tb, x + 3, 3, taskw - 5, WINHEIGHT - 6);
		set_foreground (3);		  /* white */
		draw_line (tb, x + 2, WINHEIGHT - 2, x + taskw - 2, WINHEIGHT - 2);
		draw_line (tb, x + taskw - 2, 2, x + taskw - 2, WINHEIGHT - 2);
		set_foreground (0);
		draw_line (tb, x + 1, 2, x + 1, WINHEIGHT - 2);
		set_foreground (4);		  /* darkest gray */
		draw_line (tb, x + 2, 2, x + taskw - 2, 2);
		draw_line (tb, x + 2, 2, x + 2, WINHEIGHT - 3);
	} else
	{
		set_foreground (tk->demands_attention ? 6 : 0);		  /* mid gray */
		fill_rect (tb, x + 2, 1, taskw - 1, WINHEIGHT - 2);
	}

	{

#ifdef DO_ICON
		register int text_x = x + TEXTPAD + TEXTPAD + ICONWIDTH;
#else
		register int text_x = x + TEXTPAD + TEXTPAD;
#endif

		/* check how many chars can fit */
		len = strlen (tk->name);
		while (XTextWidth (xfs, tk->name, len) >= taskw - (text_x - x) - 2
				 && len > 0)
			len--;

		if (tk->iconified)
		{
			/* draw task's name dark (iconified) */
			set_foreground (3);
			XDrawString (dd, tb->win, fore_gc, text_x, text_y + 1, tk->name,
							 len);
			set_foreground (4);
		} else
		{
			set_foreground (5);
		}

		/* draw task's name here */
		XDrawString (dd, tb->win, fore_gc, text_x, text_y, tk->name, len);
	}

#ifndef HAVE_XPM
	if (!tk->icon)
		return;
#endif

	/* draw the task's icon */
#ifdef DO_ICON        
	XSetClipMask (dd, fore_gc, tk->mask);
	XSetClipOrigin (dd, fore_gc, x + TEXTPAD, (WINHEIGHT - ICONHEIGHT) / 2);
	XCopyArea (dd, tk->icon, tb->win, fore_gc, 0, 0, ICONWIDTH, ICONHEIGHT,
				  x + TEXTPAD, (WINHEIGHT - ICONHEIGHT) / 2);
	XSetClipMask (dd, fore_gc, None);
#endif
}

void gui_draw_clock (taskbar * tb){
	char *time_str;
	time_t now;
	int width, old_x, x = WINWIDTH - time_width - (TEXTPAD * 4);
//	int width, old_x, x = WINWIDTH - time_width - (TEXTPAD * 2);

	old_x = x;

	width = WINWIDTH - x - 2;

	now = time (0);
	time_str = ctime (&now) + 11;

	//gui_draw_vline (tb, x);
	x += TEXTPAD;
#if 0
set_foreground (3); /* white *//* it's already 3 from gui_draw_vline() */
	draw_line (tb, x + 1, WINHEIGHT - 2, old_x + width - TEXTPAD,
				  WINHEIGHT - 2);
	draw_line (tb, old_x + width - TEXTPAD, 2, old_x + width - TEXTPAD,
				  WINHEIGHT - 2);

#endif
	set_foreground (1);			  /* mid gray */
	fill_rect (tb, x + 1, 2, width - (TEXTPAD * 2) - 1, WINHEIGHT - 4);
#if 0
	set_foreground (4);			  /* darkest gray */
	draw_line (tb, x, 2, x + width - (TEXTPAD * 2) - 1, 2);
	draw_line (tb, x, 2, x, WINHEIGHT - 2);
#endif
	set_foreground (5);
	XDrawString (dd, tb->win, fore_gc, x + TEXTPAD - 1, text_y, 
						time_str, 5);
}

void gui_draw_taskbar (taskbar * tb){
	task *tk;
	int x, width, taskw;
	int under = 0;

	set_foreground (5);	/* black */

	width = WINWIDTH - time_width - (TEXTPAD * 4);
	x = 0;

	if (tb->num_tasks == 0)
		goto clear;

	taskw = width / tb->num_tasks;
	if (taskw > MAX_TASK_WIDTH)
	{
		taskw = MAX_TASK_WIDTH;
		under = 1;
	}

	tk = tb->task_list;
	while (tk)
	{
		tk->pos_x = x;
		tk->width = taskw - 1;
		gui_draw_task (tb, tk);
		x += taskw;
		tk = tk->next;
	}

	if (under)
	{
		gui_draw_vline (tb, x);
clear:
		set_foreground (0);
		fill_rect (tb, x + 2, 0, WINWIDTH, WINHEIGHT);
	}

	gui_draw_clock (tb);


}

task * find_task (taskbar * tb, Window win){
	task *list = tb->task_list;
	while (list)
	{
		if (list->win == win)
			return list;
		list = list->next;
	}
	return 0;
}

void del_task (taskbar * tb, Window win){
	task *next, *prev = 0, *list = tb->task_list;

	while (list)
	{
		next = list->next;
		if (list->win == win)
		{
			/* unlink and free this task */
			tb->num_tasks--;
			if (list->icon_copied)
			{
				XFreePixmap (dd, list->icon);
				if (list->mask != None)
					XFreePixmap (dd, list->mask);
			}
			if (list->name)
				XFree (list->name);
			free (list);
			if (prev == 0)
				tb->task_list = next;
			else
				prev->next = next;
			return;
		}
		prev = list;
		list = next;
	}
}

void taskbar_read_clientlist (taskbar * tb){
	Window *win, focus_win;
	int num, i, rev, desk, new_desk = 0;
	task *list, *next;

	desk = find_desktop (root_win);
	if (desk != tb->my_desktop)
	{
		new_desk = 1;
		tb->my_desktop = desk;
	}

	XGetInputFocus (dd, &focus_win, &rev);

	/* try unified window spec first */
	win = get_prop_data (root_win, atom__NET_CLIENT_LIST, XA_WINDOW, &num);
	if (!win)
		return;

	/* remove windows that arn't in the _WIN_CLIENT_LIST anymore */
	list = tb->task_list;
	while (list)
	{
		list->focused = (focus_win == list->win);
		next = list->next;

		if (!new_desk)
			for (i = num - 1; i >= 0; i--)
				if (list->win == win[i])
					goto dontdel;
		del_task (tb, list->win);
dontdel:

		list = next;
	}

	/* add any new windows */
	for (i = 0; i < num; i++)
	{
		if (!find_task (tb, win[i]))
			add_task (tb, win[i], (win[i] == focus_win));
	}

	XFree (win);
}

void handle_press (taskbar * tb, int x, int y, int button){
	task *tk;

	tk = tb->task_list;
	while (tk)
	{
		if (x > tk->pos_x && x < tk->pos_x + tk->width)
		{
  switch(button){
    case 1:
		// The next commented stuff is needed if you care about
		// having to double click if you iconify something and
		// don't switch focus to something else
		//	if (tk->iconified)
		//	{
		//		tk->iconified = 0;
		//		tk->focused = 1;
 //   XMapRaised(dd, tk->win);


///			} else
			{
				if (tk->focused) {
//					tk->iconified = 1;
					tk->focused = 0;
					XLowerWindow (dd, tk->win);
//					XIconifyWindow (dd, tk->win, scr_screen);
				} else {
					tk->focused = 1;
					client_msg(tk->win, atom__NET_ACTIVE_WINDOW, 0, 0, 0, 0, 0);
					XSetInputFocus (dd, tk->win, RevertToNone, CurrentTime);
				}
			}
    break;
/*
    case 2: // middle button
					tk->focused = 1;
					XRaiseWindow (dd, tk->win);
    break;
*/
    case 3: // right button
	client_msg(tk->win, atom__NET_CLOSE_WINDOW, 0, 0, 0, 0, 0);
    break;
/*
    case 4: // scroll

		if(tk->next != NULL){
			task* n = tk->next;
			tk->next = n->next;
			n->next = tk->next;
		}
    break;
    case 5: // scroll

    break;
*/
  }
			gui_sync ();
			gui_draw_task (tb, tk);
		} else {
			if (tk->focused){
				tk->focused = 0;
				gui_draw_task (tb, tk);
			}
		}

		tk = tk->next;
	}
}

void handle_focusin (taskbar * tb, Window win){
	task *tk;

	tk = tb->task_list;
	while (tk){
		if (tk->focused){
			if (tk->win != win){
				tk->focused = 0;
				gui_draw_task (tb, tk);
			}
		} else {
			if (tk->win == win){
				tk->focused = 1;
				gui_draw_task (tb, tk);
			}
		}
		tk = tk->next;
	}
}

void handle_propertynotify (taskbar * tb, Window win, Atom at){
	task *tk;

	if (win == root_win){
		if (at == atom__NET_CLIENT_LIST || at == atom__NET_CURRENT_DESKTOP){
			taskbar_read_clientlist (tb);
			gui_draw_taskbar (tb);
		}
		return;
	}

	tk = find_task (tb, win);
	if (!tk){
		if (at == atom__NET_WM_STATE){
		if(!is_iconified(win)){
			add_task(tb, win, 1);
			gui_draw_taskbar (tb);
		}}
/*// I think this is impossible
		else if(at == atom__NET_CURRENT_DESKTOP){
			unsigned long desk;
			desk = find_desktop(win);
			if(desk == 0xffffffff || desk == tb->my_desktop){
				add_task(tb, win, 1);
				gui_draw_taskbar (tb);
			}
		}
*/
		return;
	}

	if (at == XA_WM_NAME)
	{
		/* window's title changed */
		if (tk->name)
			XFree (tk->name);
		tk->name = get_prop_data (tk->win, XA_WM_NAME, XA_STRING, 0);
		gui_draw_task (tb, tk);
	} else if (at == atom__NET_WM_STATE)
	{
		/* iconified state changed? */

		unsigned long state = get_state(tk->win);
		if((state & STATE_ICONIFIED) && !(state & STATE_SHADED)){
			del_task(tb, tk->win);
			gui_draw_taskbar (tb);
		}

		if(!!(state & STATE_SHADED) != tk->iconified){
			tk->iconified = !tk->iconified;
			gui_draw_task (tb, tk);
		}

		if(!!(state & STATE_DEMANDS_ATTENTION) != tk->demands_attention){
			tk->demands_attention = !tk->demands_attention;
			gui_draw_task (tb, tk);
		}

	} else if (at == atom__NET_WM_DESKTOP){
		// Virtual desktop switch
		int desk = find_desktop(tk->win);
		if(desk != 0xffffffff && desk != tb->my_desktop){
			del_task(tb, tk->win);
			gui_draw_taskbar (tb);
		}
	}
#ifdef DO_ICON
        else if (at == XA_WM_HINTS)
	{	// Icon update
		get_task_hinticon (tk);
		gui_draw_task (tb, tk);
	}
#endif
}

void handle_error (Display * d, XErrorEvent * ev){
}

void grab_keys (Display * d) {
	int i;
	int min, max;
	KeyCode keycode;
	int modifier = Mod4Mask;

	XDisplayKeycodes (d, &min, &max);

	for(i = 0; i <= 9; i++) {
		keycode = 10 + i;
		XGrabKey (d, keycode, modifier, DefaultRootWindow (d),
			  False, GrabModeAsync, GrabModeAsync);
	}

}

task * select_task(taskbar *tb, int idx) {
	task *tk;
	int i = 0;

	tk = tb->task_list;

	if (idx > tb->num_tasks) {
		return 0;
	}

	while (i != idx && tk) {
		tk = tk->next;
		i++;
	}
	
	return tk;

}

void handle_key (taskbar *tb, KeyCode keycode) {
	task *tk;

	tk = select_task (tb, keycode - 10);
	if (tk) {
		tk->focused = 1;
		client_msg(tk->win, atom__NET_ACTIVE_WINDOW, 0, 0, 0, 0, 0);
		XSetInputFocus (dd, tk->win, RevertToNone, CurrentTime);
		gui_draw_task (tb, tk);
	}
}

int
#ifdef NOSTDLIB
_start (void)
#else
main (int argc, char *argv[])
#endif
{
	taskbar *tb;
	XEvent ev;
	fd_set fd;
	struct timeval tv;
	int xfd;
	time_t now;
	struct tm *lt;

	dd = XOpenDisplay (NULL);
	if (!dd)
		return 0;
	scr_screen = DefaultScreen (dd);
	scr_depth = DefaultDepth (dd, scr_screen);
	scr_height = DisplayHeight (dd, scr_screen);
	scr_width = DisplayWidth (dd, scr_screen);
	root_win = RootWindow (dd, scr_screen);

	/* helps us catch windows closing/opening */
	XSelectInput (dd, root_win, PropertyChangeMask);

	XSetErrorHandler ((XErrorHandler) handle_error);

	XInternAtoms (dd, atom_names, ATOM_COUNT, False, atoms);

	gui_init ();
	tb = gui_create_taskbar ();
	xfd = ConnectionNumber (dd);
	gui_sync ();
	grab_keys (dd);

	while (1)
	{
		now = time (0);
		lt = gmtime (&now);
		tv.tv_usec = 0;
		tv.tv_sec = 60 - lt->tm_sec;
		FD_ZERO (&fd);
		FD_SET (xfd, &fd);
		if (select (xfd + 1, &fd, 0, 0, &tv) == 0)
			gui_draw_clock (tb);

		while (XPending (dd))
		{
			XNextEvent (dd, &ev);
			switch (ev.type)
			{
			case KeyPress:
				handle_key (tb, ev.xkey.keycode);
				break;
			case ButtonPress:
				handle_press (tb, ev.xbutton.x, ev.xbutton.y, ev.xbutton.button);
				break;
			case DestroyNotify:
				del_task (tb, ev.xdestroywindow.window);
				/* fall through */
			case Expose:
				gui_draw_taskbar (tb);
				break;
			case PropertyNotify:
				handle_propertynotify (tb, ev.xproperty.window,
											  ev.xproperty.atom);
				break;
			case FocusIn:
				handle_focusin (tb, ev.xfocus.window);
				break;
			/*default:
				   printf ("unknown evt type: %d\n", ev.type);*/
			}
		}
	}

	/*XCloseDisplay (dd);

   return 0;*/
}

/*
Local Variables:
mode: c
indent-tabs-mode: t
c-basic-offset: 8
End:
*/
