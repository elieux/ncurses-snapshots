/****************************************************************************
 * Copyright (c) 1998-2007,2008 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Juergen Pfeifer                                                 *
 *                                                                          *
 ****************************************************************************/

#if defined(__MINGW32__)
#include <curses.priv.h>

#define WINVER 0x0501
#include <windows.h>

MODULE_ID("$Id: win_driver.c,v 0.1.1.1 2008/11/18 08:50:04 juergen Exp $")


static bool   drv_CanHandle(TERMINAL_CONTROL_BLOCK* TCB, const char* tname, int*);
static void   drv_init(TERMINAL_CONTROL_BLOCK*);
static void   drv_release(TERMINAL_CONTROL_BLOCK*);
static int    drv_size(TERMINAL_CONTROL_BLOCK*,int*,int*);
static int    drv_sgmode(TERMINAL_CONTROL_BLOCK* TCB, 
		     bool setFlag, 
		     TTY* buf);
static chtype drv_conattr(TERMINAL_CONTROL_BLOCK* TCB);

static int    drv_mvcur(TERMINAL_CONTROL_BLOCK* TCB,
		  int yold,int xold,int ynew, int xnew);
static int    drv_mode(TERMINAL_CONTROL_BLOCK* TCB, 
		       bool progFlag, bool defFlag);
static bool   drv_rescol(TERMINAL_CONTROL_BLOCK* TCB);
static bool   drv_rescolors(TERMINAL_CONTROL_BLOCK* TCB);
static void   drv_setcolor(TERMINAL_CONTROL_BLOCK* TCB, 
			   bool fore, 
			   int color, 
			   int (*outc)(SCREEN*,int));
static int    drv_dobeepflash(TERMINAL_CONTROL_BLOCK* TCB,bool);
static void   drv_initpair(TERMINAL_CONTROL_BLOCK* TCB,
		      short pair, 
		      short f, 
		      short b);
static void   drv_initcolor(TERMINAL_CONTROL_BLOCK* TCB,
			    short color, 
			    short r, 
			    short g, 
			    short b);
static void   drv_do_color(TERMINAL_CONTROL_BLOCK* TCB, 
			   short old_pair, 
			   short pair, 
			   bool reverse,
			   int (*outc)(SCREEN*,int));
static void   drv_initmouse(TERMINAL_CONTROL_BLOCK* TCB);
static void   drv_setfilter(TERMINAL_CONTROL_BLOCK* TCB);
static void   drv_hwlabel(TERMINAL_CONTROL_BLOCK* TCB,
			  int labnum, 
			  char* text);
static void   drv_hwlabelOnOff(TERMINAL_CONTROL_BLOCK* TCB, 
			       bool OnFlag);
static int    drv_doupdate(TERMINAL_CONTROL_BLOCK* TCB);

static int    drv_defaultcolors(TERMINAL_CONTROL_BLOCK* TCB, 
				int fg, 
				int bg);
static int    drv_print(TERMINAL_CONTROL_BLOCK* TCB, 
			char* data, 
			int len);
/*static int _getsize(TERMINAL_CONTROL_BLOCK*,int *, int *);*/
static int    drv_setsize(TERMINAL_CONTROL_BLOCK* TCB,int l, int c);
static void   drv_initacs(TERMINAL_CONTROL_BLOCK* TCB,chtype*,chtype*);
static void   drv_wrap(SCREEN*);
static void   drv_screen_init(SCREEN*);
static int    drv_twait(TERMINAL_CONTROL_BLOCK*, 
			int, 
			int, 
			int* EVENTLIST_2nd(_nc_eventlist*));
static int    drv_read(TERMINAL_CONTROL_BLOCK* TCB, int* buf);
static int    drv_nap(TERMINAL_CONTROL_BLOCK* TCB GCC_UNUSED , int ms);
static int    drv_kpad(TERMINAL_CONTROL_BLOCK* TCB, bool);
static int    drv_keyok(TERMINAL_CONTROL_BLOCK* TCB, int, bool);
static bool   drv_kyExist(TERMINAL_CONTROL_BLOCK* TCB, int);


NCURSES_EXPORT_VAR(TERM_DRIVER) _nc_WIN_DRIVER = {
  drv_CanHandle,     /* CanHandle */
  drv_init,         /* init */
  drv_release,      /* release */
  drv_size,         /* size */
  drv_sgmode,        /* sgmode */
  drv_conattr,       /* conattr */
  drv_mvcur,        /* hwcur */
  drv_mode,         /* mode */
  drv_rescol,        /* rescol */
  drv_rescolors,     /* rescolors */
  drv_setcolor,     /* color */
  drv_dobeepflash,   /* DoBeepFlash */
  drv_initpair,     /* initpair */
  drv_initcolor,     /* initcolor */
  drv_do_color,      /* docolor */
  drv_initmouse,     /* initmouse */
  drv_setfilter,     /* setfilter */
  drv_hwlabel,       /* hwlabel */
  drv_hwlabelOnOff,  /* hwlabelOnOff */
  drv_doupdate,     /* update */
  drv_defaultcolors, /* defaultcolors */
  drv_print,        /* print */
  drv_size,         /* getsize */
  drv_setsize,      /* setsize */
  drv_initacs,       /* initacs */
  drv_screen_init,   /* scinit */
  drv_wrap,          /* scexit */
  drv_twait,        /* twait */
  drv_read,       /* read */
  drv_nap,          /* nap */
  drv_kpad,         /* kpad */
  drv_keyok,        /* kyOk */
  drv_kyExist        /* kyExist */
};

#define WINMAGIC NCDRV_MAGIC(NCDRV_WINCONSOLE)

#define AssertTCB() assert(TCB!=0 && TCB->magic==WINMAGIC)
#define SetSP() assert(TCB->csp!=0); sp = TCB->csp

#define GenMap(vKey,key) MAKELONG(key, vKey)
static const LONG keylist[] = 
  {
    GenMap(VK_PRIOR , KEY_PPAGE),
    GenMap(VK_NEXT  , KEY_NPAGE),
    GenMap(VK_END   , KEY_END  ),
    GenMap(VK_HOME  , KEY_HOME ),
    GenMap(VK_LEFT  , KEY_LEFT ),
    GenMap(VK_UP    , KEY_UP   ),
    GenMap(VK_RIGHT , KEY_RIGHT),
    GenMap(VK_DOWN  , KEY_DOWN ),
    GenMap(VK_DELETE, KEY_DC   ),
    GenMap(VK_INSERT, KEY_IC   )
  };
#define N_INI ((int)(sizeof(keylist)/sizeof(keylist[0])))
#define FKEYS 24
#define MAPSIZE (FKEYS + N_INI)
#define NUMPAIRS 64

typedef struct props
{
  CONSOLE_SCREEN_BUFFER_INFO SBI;
  bool progMode;
  DWORD map[MAPSIZE];
  DWORD rmap[MAPSIZE];
  WORD pairs[NUMPAIRS];
} Properties;

#define PropOf(TCB) ((Properties*)TCB->prop)

int _nc_mingw_ioctl(int fd GCC_UNUSED, 
		    long int request GCC_UNUSED, 
		    struct termios* arg GCC_UNUSED)
{
  return 0;
  endwin();
  fprintf(stderr,"TERMINFO currently not supported on Windows.\n");
  exit(1);
}
 
static WORD 
MapColor(bool fore, int color)
{
  static const int _cmap[] = { 0, 4, 2, 6, 1, 5, 3, 7 };
  int a;
  if (color < 0 || color > 7)
    a = fore ? 7 : 0;
  else
    a = _cmap[color];
  if (!fore)
    a = a<<4;
  return (WORD)a;
}

static WORD
MapAttr(TERMINAL_CONTROL_BLOCK* TCB, WORD res, chtype ch)
{
  if (ch & A_COLOR)
  {
      int p;
      SCREEN* sp;

      AssertTCB();
      SetSP();
      p = PAIR_NUMBER(ch);
      if (p>0 && p<NUMPAIRS && TCB!=0 && sp!=0)
      {
	  WORD a;
	  a = PropOf(TCB)->pairs[p];
	  res = (res&0xff00)|a;
      }
  }

  if (ch & A_REVERSE)
    res = ((res&0xff00) | (((res&0x07)<<4) | ((res&0x70)>>4)));    

  if (ch & A_STANDOUT)
    res = ((res&0xff00) | (((res&0x07)<<4) | ((res&0x70)>>4)) 
	   | BACKGROUND_INTENSITY);    

  if (ch & A_BOLD)
    res |= FOREGROUND_INTENSITY;

  if (ch & A_DIM)
    res |= BACKGROUND_INTENSITY;

  return res;
}

static BOOL 
con_write(TERMINAL_CONTROL_BLOCK* TCB, int y, int x, chtype* str, int n)
{
  CHAR_INFO ci[n];
  COORD loc,siz;
  SMALL_RECT rec;
  int i;
  chtype ch;
  SCREEN* sp;

  AssertTCB();

  if (TCB==0 || InvalidConsoleHandle(TCB->hdl))
    return FALSE;
  
  SetSP();

  for(i=0; i < n; i++)
  {
      ch = str[i];
      ci[i].Char.AsciiChar = ChCharOf(ch);
      ci[i].Attributes     = MapAttr(TCB,
				     PropOf(TCB)->SBI.wAttributes,
				     ChAttrOf(ch));
      if (ChAttrOf(ch) & A_ALTCHARSET)
      {
	if (sp->_acs_map)
	  ci[i].Char.AsciiChar = 
	    ChCharOf(NC_SNAME(_nc_acs_char)(sp,ChCharOf(ch)));
      }
  }

  loc.X = (short)0; 
  loc.Y = (short)0;
  siz.X = (short)n; 
  siz.Y = 1;

  rec.Left   = (short)x; 
  rec.Top    = (short)y;
  rec.Right  = (short)(x+n-1) ;
  rec.Bottom = rec.Top;

  return WriteConsoleOutput(TCB->hdl, ci, siz, loc, &rec);
}

#define MARK_NOCHANGE(win,row) \
		win->_line[row].firstchar = _NOCHANGE; \
		win->_line[row].lastchar  = _NOCHANGE

static int 
drv_doupdate(TERMINAL_CONTROL_BLOCK* TCB)
{
  int y, nonempty,n,x0,x1, Width, Height;
  SCREEN* sp;

  AssertTCB();
  SetSP();

  Width  = screen_columns(sp);
  Height = screen_lines(sp);
  nonempty = min(Height, sp->_newscr->_maxy + 1);

  if ((sp->_curscr->_clear || sp->_newscr->_clear)) 
  {
      int x;
      chtype empty[Width];
      
      for (x=0; x < Width; x++)
	empty[x] = ' ';

      for (y = 0; y < nonempty; y++) {
	con_write(TCB,y,0,empty,Width);
	memcpy(empty,
	       sp->_curscr->_line[y].text,
	       Width * sizeof(chtype));
      }
      sp->_curscr->_clear = FALSE;
      sp->_newscr->_clear = FALSE;
      touchwin(sp->_newscr);
  } 
      
  for(y=0; y < nonempty; y++)
    {
      x0 = sp->_newscr->_line[y].firstchar;
      if (x0 != _NOCHANGE)
	{
	  x1 = sp->_newscr->_line[y].lastchar;
	  n = x1 - x0 + 1;
	  if (n>0)
	    {
	      memcpy(sp->_curscr->_line[y].text +x0,
		     sp->_newscr->_line[y].text +x0,
		     n*sizeof(chtype));
	      con_write(TCB,
			y,
			x0,
			((chtype*)sp->_curscr->_line[y].text) + x0, n);
	      
	      /* mark line changed successfully */
	      if (y <= sp->_newscr->_maxy) {
		MARK_NOCHANGE(sp->_newscr, y);
	      }
	      if (y <= sp->_curscr->_maxy) {
		MARK_NOCHANGE(sp->_curscr, y);
	      }
	    }
	}
    }

    /* put everything back in sync */
    for (y = nonempty; y <= sp->_newscr->_maxy; y++) {
	MARK_NOCHANGE(sp->_newscr, y);
    }
    for (y = nonempty; y <= sp->_curscr->_maxy; y++) {
	MARK_NOCHANGE(sp->_curscr, y);
    }

    if (!sp->_newscr->_leaveok) {
	sp->_curscr->_curx = sp->_newscr->_curx;
	sp->_curscr->_cury = sp->_newscr->_cury;

	TCB->drv->hwcur(TCB,0,0,sp->_curscr->_cury, sp->_curscr->_curx);
    }
    SetConsoleActiveScreenBuffer(TCB->hdl);
    return OK;
}

static bool 
drv_CanHandle(TERMINAL_CONTROL_BLOCK* TCB, 
	      const char* tname,
	      int* errret GCC_UNUSED)
{
  assert(TCB!=0);
  assert(tname!=0);
  TCB->magic = WINMAGIC;
  if (*tname==0 || *tname==0 || strcmp(tname,"unknown")==0)
  {
    return TRUE;
  }
  else
    return FALSE;
}

static int 
drv_dobeepflash(TERMINAL_CONTROL_BLOCK* TCB, 
		bool beepFlag GCC_UNUSED)
{
  SCREEN* sp;
  int res = ERR;

  AssertTCB();
  SetSP();

  return res;
}


static int 
drv_print(TERMINAL_CONTROL_BLOCK* TCB, 
	  char* data GCC_UNUSED, 
	  int len GCC_UNUSED)
{
  SCREEN* sp;

  AssertTCB();
  SetSP();

  return ERR;
}

static int 
drv_defaultcolors(TERMINAL_CONTROL_BLOCK* TCB,
		  int fg GCC_UNUSED, 
		  int bg GCC_UNUSED)
{
  SCREEN* sp;
  int code = ERR;
  
  AssertTCB();
  SetSP();

  return(code);
}

static void 
drv_setcolor(TERMINAL_CONTROL_BLOCK* TCB,
	     bool fore, 
	     int color, 
	     int(*outc)(SCREEN*,int) GCC_UNUSED)
{
  AssertTCB();

  if (TCB && !InvalidConsoleHandle(TCB->hdl)) {
    WORD a = MapColor(fore,color);
    a = ((PropOf(TCB)->SBI.wAttributes) & (fore?0xfff8:0xff8f)) | a;
    SetConsoleTextAttribute(TCB->hdl,a);
    GetConsoleScreenBufferInfo(TCB->hdl,&(PropOf(TCB)->SBI));
  }
}

static bool 
drv_rescol(TERMINAL_CONTROL_BLOCK* TCB)
{
  bool res = FALSE;

  AssertTCB();
  if (TCB && !InvalidConsoleHandle(TCB->hdl)) {
    WORD a = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
    SetConsoleTextAttribute(TCB->hdl, a);
    GetConsoleScreenBufferInfo(TCB->hdl,&(PropOf(TCB)->SBI));
    res = TRUE;
  }
  return res;
}

static bool 
drv_rescolors(TERMINAL_CONTROL_BLOCK* TCB)
{
  int result = FALSE;
  SCREEN* sp;
  
  AssertTCB();
  SetSP();

  return result;
}

static int 
drv_size(TERMINAL_CONTROL_BLOCK* TCB, int* Lines, int *Cols)
{
  AssertTCB();
  
  if (TCB==NULL || Lines==NULL || Cols==NULL || InvalidConsoleHandle(TCB->hdl))
    return ERR;
  
  *Lines = (int)(PropOf(TCB)->SBI.dwSize.Y);
  *Cols  = (int)(PropOf(TCB)->SBI.dwSize.X);
  return OK;
}

static int 
drv_setsize(TERMINAL_CONTROL_BLOCK* TCB,
	    int l GCC_UNUSED, 
	    int c GCC_UNUSED)
{
  AssertTCB();
  return ERR;
}


static int 
drv_mode(TERMINAL_CONTROL_BLOCK* TCB, bool progFlag, bool defFlag)
{
  SCREEN *sp;
  TERMINAL* _term = (TERMINAL*)TCB;
  int code = ERR;

  AssertTCB();
  sp = TCB->csp;

  PropOf(TCB)->progMode = progFlag;
  SetConsoleActiveScreenBuffer(progFlag ? TCB->hdl : TCB->out);

  if (progFlag) /* prog mode */ {
    if (defFlag) {
      if ((drv_sgmode(TCB, FALSE, &(_term->Nttyb)) == OK)) {
	_term->Nttyb.c_oflag &= ~OFLAGS_TABS;
	code = OK;
      }
    }
    else {
      /* reset_prog_mode */
      if (drv_sgmode(TCB, TRUE, &(_term->Nttyb)) == OK) {
	if (sp) {
	  if (sp->_keypad_on)
	    _nc_keypad(sp, TRUE);
	  NC_BUFFERED(sp, TRUE);
	}
	code = OK;
      }
    }
  }
  else /* shell mode */ {
    if (defFlag)
      {
	/* def_shell_mode */
	if (drv_sgmode(TCB, FALSE, &(_term->Ottyb)) == OK) {
	  code = OK;
	}	
      }
    else {
      /* reset_shell_mode */
      if (sp) {
	_nc_keypad(sp, FALSE);
	NC_SNAME(_nc_flush)(sp);
	NC_BUFFERED(sp, FALSE);
      }
      code = drv_sgmode(TCB, TRUE, &(_term->Ottyb));
    }
  }
  
  return(code);
}

static void 
drv_screen_init(SCREEN* sp GCC_UNUSED)
{
}

static void 
drv_wrap(SCREEN* sp GCC_UNUSED)
{
}

static int 
rkeycompare(const void* el1, const void* el2)
{
  WORD key1 = (LOWORD((*((const LONG*)el1)))) & 0x7fff;
  WORD key2 = (LOWORD((*((const LONG*)el2)))) & 0x7fff;

  return ((key1 < key2) ? -1 : ((key1 == key2) ? 0 : 1));
}

static int 
keycompare(const void* el1, const void* el2)
{
  WORD key1 = HIWORD((*((const LONG*)el1)));
  WORD key2 = HIWORD((*((const LONG*)el2)));

  return ((key1 < key2) ? -1 : ((key1 == key2) ? 0 : 1));
}


static int 
MapKey(TERMINAL_CONTROL_BLOCK* TCB, WORD vKey)
{
  WORD nKey = 0;
  void* res;
  LONG key = GenMap(vKey,0);
  int  code = -1;

  AssertTCB();

  res = bsearch(&key, 
		PropOf(TCB)->map, 
		(size_t)(N_INI + FKEYS), 
		sizeof(keylist[0]), 
		keycompare);
  if (res) {
    key  = *((LONG*)res);
    nKey = LOWORD(key);
    code = (int)(nKey & 0x7fff);
    if (nKey & 0x8000)
      code = -code;
  }
  return code;
}

static void 
drv_release(TERMINAL_CONTROL_BLOCK* TCB)
{
  AssertTCB();
  if (TCB->info)
    free(TCB->info);
  if (TCB->prop)
    free(TCB->prop);
}

static void 
drv_init(TERMINAL_CONTROL_BLOCK* TCB)
{
  AssertTCB();

  if (TCB) {
    BOOL b = AllocConsole();
    WORD a;
    
    if (!b)
      b = AttachConsole(ATTACH_PARENT_PROCESS);
    
    TCB->inp  = GetStdHandle(STD_INPUT_HANDLE);
    TCB->out  = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if (getenv("NCGDB"))
      TCB->hdl = TCB->out;
    else
      TCB->hdl  = CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,
					    0,
					    NULL,
					    CONSOLE_TEXTMODE_BUFFER,
					    NULL);
    
    if (!InvalidConsoleHandle(TCB->hdl)) {
      TCB->prop = typeCalloc(Properties,1);
      GetConsoleScreenBufferInfo(TCB->hdl,&(PropOf(TCB)->SBI));
    }
    
    TCB->info = typeCalloc(TerminalInfo,1);
    if (TCB->info) {
      int i;
      
      TCB->info->initcolor = TRUE;
      TCB->info->canchange = FALSE;
      TCB->info->hascolor  = TRUE;  
      TCB->info->caninit   = TRUE;  
      
      TCB->info->maxpairs      = NUMPAIRS;
      TCB->info->maxcolors     = 8;
      TCB->info->numlabels     = 0;
      TCB->info->labelwidth    = 0;
      TCB->info->labelheight   = 0;
      TCB->info->nocolorvideo  = 1;
      TCB->info->tabsize       = 8;
      
      TCB->info->defaultPalette = _nc_cga_palette;
      
      for(i=0; i < (N_INI + FKEYS); i++) {
	if (i<N_INI)
	  PropOf(TCB)->rmap[i] = PropOf(TCB)->map[i] = keylist[i];
	else
	  PropOf(TCB)->rmap[i] = PropOf(TCB)->map[i] =
	    GenMap((VK_F1+(i-N_INI)),(KEY_F(1)+(i-N_INI)));
      }
      qsort(PropOf(TCB)->map, 
	    (size_t)(MAPSIZE), 
	    sizeof(keylist[0]), 
	    keycompare);
      qsort(PropOf(TCB)->rmap, 
	    (size_t)(MAPSIZE), 
	    sizeof(keylist[0]), 
	    rkeycompare);
      
      a = MapColor(true,COLOR_WHITE) | MapColor(false,COLOR_BLACK);
      for(i=0; i < NUMPAIRS; i++)
	PropOf(TCB)->pairs[i] = a;
    }
  }
}

static void 
drv_initpair(TERMINAL_CONTROL_BLOCK* TCB,
	     short pair, 
	     short f, 
	     short b)
{
  SCREEN* sp;
  
  AssertTCB();
  SetSP();

  if ((pair>0) && (pair < NUMPAIRS) && (f>=0) && (f<8)
      && (b>=0) && (b<8)) {
    PropOf(TCB)->pairs[pair] = MapColor(true,f) | MapColor(false,b);
  }
}

static void 
drv_initcolor(TERMINAL_CONTROL_BLOCK* TCB,
	      short color GCC_UNUSED,
	      short r GCC_UNUSED, 
	      short g GCC_UNUSED,
	      short b GCC_UNUSED)
{
  SCREEN* sp;
  
  AssertTCB();
  SetSP();
}

static void 
drv_do_color(TERMINAL_CONTROL_BLOCK* TCB,
	     short old_pair GCC_UNUSED, 
	     short pair GCC_UNUSED, 
	     bool reverse GCC_UNUSED,
	     int (*outc)(SCREEN*,int) GCC_UNUSED
	     )
{
  SCREEN* sp;
  
  AssertTCB();
  SetSP();
}

static void 
drv_initmouse(TERMINAL_CONTROL_BLOCK* TCB)
{
  SCREEN* sp;
  
  AssertTCB();
  SetSP();
}

static int 
drv_mvcur(TERMINAL_CONTROL_BLOCK* TCB, 
	  int yold GCC_UNUSED, int xold GCC_UNUSED, 
	  int y, int x)
{
  int ret = ERR;
  if (TCB && !InvalidConsoleHandle(TCB->hdl)) {
    COORD loc;
    loc.X = (short)x;
    loc.Y = (short)y;
    SetConsoleCursorPosition(TCB->hdl,loc);
    ret = OK;
  }
  return ret;
}

static void 
drv_hwlabel(TERMINAL_CONTROL_BLOCK* TCB,
	    int labnum GCC_UNUSED, 
	    char* text GCC_UNUSED)
{
  SCREEN* sp;
  
  AssertTCB();
  SetSP();
}

static void 
drv_hwlabelOnOff(TERMINAL_CONTROL_BLOCK* TCB, 
		 bool OnFlag GCC_UNUSED)
{
  SCREEN* sp;
  
  AssertTCB();
  SetSP();
}

static chtype 
drv_conattr(TERMINAL_CONTROL_BLOCK* TCB GCC_UNUSED)
{
  chtype res = A_NORMAL;
  res |= (A_BOLD | A_DIM | A_REVERSE | A_STANDOUT | A_COLOR);
  return res;
}

static void 
drv_setfilter(TERMINAL_CONTROL_BLOCK* TCB)
{
  SCREEN* sp;
  
  AssertTCB();
  SetSP();
}

static int 
drv_sgmode(TERMINAL_CONTROL_BLOCK* TCB, bool setFlag, TTY* buf)
{
  DWORD dwFlag = 0;
  tcflag_t iflag;
  tcflag_t lflag;

  AssertTCB();

  if (TCB==0 || buf==NULL)
      return ERR;

  if (setFlag) {
    iflag = buf->c_iflag;
    lflag = buf->c_lflag;
    
    GetConsoleMode(TCB->inp, &dwFlag);
    
    if (lflag & ICANON)
	dwFlag |= ENABLE_LINE_INPUT;
    else
      dwFlag &= ~ENABLE_LINE_INPUT; 
    
    if (lflag & ECHO)
      dwFlag |= ENABLE_ECHO_INPUT;
    else
      dwFlag &= ~ENABLE_ECHO_INPUT;
    
    if (iflag & BRKINT)
      dwFlag |= ENABLE_PROCESSED_INPUT;
    else
      dwFlag &= ~ENABLE_PROCESSED_INPUT;
    
    /* we disable that for now to focus on keyboard. */
    dwFlag &= ~ENABLE_MOUSE_INPUT;
    
    buf->c_iflag = iflag;
    buf->c_lflag = lflag;
    SetConsoleMode(TCB->inp, dwFlag);
    TCB->term.Nttyb = *buf;
  }
  else {
    iflag = TCB->term.Nttyb.c_iflag;
    lflag = TCB->term.Nttyb.c_lflag;
    GetConsoleMode(TCB->inp, &dwFlag);
    
    if (dwFlag & ENABLE_LINE_INPUT)
      lflag |= ICANON;
    else
      lflag &= ~ICANON;
    
    if (dwFlag & ENABLE_ECHO_INPUT)
      lflag |= ECHO;
    else
      lflag &= ~ECHO;
    
    if (dwFlag & ENABLE_PROCESSED_INPUT)
      iflag |= BRKINT;
    else
      iflag &= ~BRKINT;
    
    TCB->term.Nttyb.c_iflag = iflag; 
    TCB->term.Nttyb.c_lflag = lflag; 
    
    *buf = TCB->term.Nttyb;
  }
  return OK;
}

static void 
drv_initacs(TERMINAL_CONTROL_BLOCK* TCB, 
	    chtype* real_map GCC_UNUSED, 
	    chtype* fake_map GCC_UNUSED)
{
  SCREEN* sp;
  AssertTCB();
  SetSP();
}

static ULONGLONG 
tdiff(FILETIME fstart, FILETIME fend)
{
  ULARGE_INTEGER ustart;
  ULARGE_INTEGER uend;
  ULONGLONG diff;

  ustart.LowPart  = fstart.dwLowDateTime;
  ustart.HighPart = fstart.dwHighDateTime;
  uend.LowPart    = fend.dwLowDateTime;
  uend.HighPart   = fend.dwHighDateTime;

  diff = (uend.QuadPart - ustart.QuadPart) / 10000;
  return diff;
}

static int 
Adjust(int milliseconds, int diff)
{
  if (milliseconds == INFINITY)
    return milliseconds;
  milliseconds -= diff;
  if (milliseconds < 0)
    milliseconds = 0;
  return milliseconds;
}

static int 
drv_twait(TERMINAL_CONTROL_BLOCK* TCB,
	  int mode,
	  int milliseconds,
	  int *timeleft
	  EVENTLIST_2nd(_nc_eventlist * evl))
{
  SCREEN* sp;
  INPUT_RECORD inp;
  BOOL b;
  DWORD nRead = 0, rc = -1;
  int code = 0;
  FILETIME fstart;
  FILETIME fend;
  int diff;
  bool isImmed = (milliseconds==0);

#define CONSUME() ReadConsoleInput(TCB->inp,&inp,1,&nRead)

  AssertTCB();
  SetSP();

  if (milliseconds < 0)
    milliseconds = INFINITY;

  memset(&inp,0,sizeof(inp));

  while(true) {
    GetSystemTimeAsFileTime(&fstart);
    rc = WaitForSingleObject(TCB->inp,milliseconds);
    GetSystemTimeAsFileTime(&fend);
    diff = (int)tdiff(fstart,fend);
    milliseconds = Adjust(milliseconds, diff);
    
    if (!isImmed && milliseconds==0)
      break;
    
    if (rc == WAIT_OBJECT_0) {
      if (mode) {
	b = GetNumberOfConsoleInputEvents(TCB->inp,&nRead);	      
	if (b && nRead > 0) {
	  b = PeekConsoleInput(TCB->inp, &inp, 1, &nRead); 
	  if (b && nRead > 0) {
	    switch(inp.EventType) {
	    case KEY_EVENT:
	      if (mode&TW_INPUT) {
		WORD vk = inp.Event.KeyEvent.wVirtualKeyCode;
		char ch = inp.Event.KeyEvent.uChar.AsciiChar;
		
		if (inp.Event.KeyEvent.bKeyDown) {
		  if (0==ch) {
		    int nKey = MapKey(TCB,vk);
		    if ((nKey<0) || FALSE==sp->_keypad_on) {
		      CONSUME();
		      continue;
		    }
		  }
		  code = TW_INPUT;
		  goto end;  
		}
		else {
		  CONSUME();			      
		}
	      }
	      continue;
	    case MOUSE_EVENT:
	      if (0 && mode & TW_MOUSE) {
		code = TW_MOUSE;
		goto end;
	      }
	      else
		continue;
	    default:
	      SetConsoleActiveScreenBuffer(!PropOf(TCB)->progMode ?
					   TCB->hdl : TCB->out);
	      continue;
	    }
	  }
	}
      }
      continue;
    }
    else {
      if (rc != WAIT_TIMEOUT) {
	code = -1;
	break;
      }
      else {
	code = 0;
	break;
      }
    }
  }
 end:  
  if (timeleft)
    *timeleft = milliseconds;
  
  return code;
}

static int 
drv_read(TERMINAL_CONTROL_BLOCK* TCB, int* buf)
{
  SCREEN* sp;
  int n = 1;
  INPUT_RECORD inp;
  BOOL b;
  DWORD nRead;
  WORD vk;
  WORD sc;

  AssertTCB();
  assert(buf);
  SetSP();

  memset(&inp,0,sizeof(inp));

  while((b = ReadConsoleInput(TCB->inp, &inp, 1, &nRead))) {
    if (b && nRead > 0) {
      if (inp.EventType == KEY_EVENT) {
	if (!inp.Event.KeyEvent.bKeyDown)
	  continue;
	*buf = (int)inp.Event.KeyEvent.uChar.AsciiChar;
	vk = inp.Event.KeyEvent.wVirtualKeyCode;
	sc = inp.Event.KeyEvent.wVirtualScanCode;
	if (*buf==0) {
	  if (sp->_keypad_on) {
	    *buf = MapKey(TCB, vk);
	    if (0 > (*buf))
	      continue;
	    else
	      break;
	  }
	  else
	    continue;
	}
	else { /* *buf != 0 */ 
	  break;
	}
      }
      else if (0 && inp.EventType == MOUSE_EVENT) {
	*buf = KEY_MOUSE;
	break;
      }
      continue;
    }
  }
  return n;	       
}

static int 
drv_nap(TERMINAL_CONTROL_BLOCK* TCB GCC_UNUSED , int ms)
{
  Sleep(ms);
  return OK;
}

static bool 
drv_kyExist(TERMINAL_CONTROL_BLOCK* TCB, int keycode)
{
  SCREEN* sp;
  WORD nKey;
  void* res;
  bool found = FALSE;
  LONG key = GenMap(0,(WORD)keycode);
  
  AssertTCB();
  SetSP();

  AssertTCB();

  res = bsearch(&key, 
		PropOf(TCB)->rmap, 
		(size_t)(N_INI + FKEYS), 
		sizeof(keylist[0]), 
		rkeycompare);
  if (res) {
    key  = *((LONG*)res);
    nKey = LOWORD(key);
    if (!(nKey&0x8000))
      found = TRUE;
  }
  return found;
}

static int 
drv_kpad(TERMINAL_CONTROL_BLOCK* TCB, bool flag GCC_UNUSED)
{
  SCREEN* sp;
  int code = ERR;

  AssertTCB();
  sp = TCB->csp;

  if (sp) {
    code = OK;
  }
  return code;
}

static int drv_keyok(TERMINAL_CONTROL_BLOCK* TCB, int keycode, bool flag)
{
  int code = ERR;
  SCREEN* sp;
  WORD nKey;
  WORD vKey;
  void* res;
  LONG key = GenMap(0,(WORD)keycode);
  
  AssertTCB();
  SetSP();

  if (sp) {
    res = bsearch(&key, 
		  PropOf(TCB)->rmap, 
		  (size_t)(N_INI + FKEYS), 
		  sizeof(keylist[0]), 
		  rkeycompare);
    if (res) {
      key  = *((LONG*)res);
      vKey = HIWORD(key);
      nKey = (LOWORD(key)) & 0x7fff;
      if (!flag)
	nKey |= 0x8000;
      *(LONG*)res = GenMap(vKey,nKey);
    }
  }
  return code;
}

#endif