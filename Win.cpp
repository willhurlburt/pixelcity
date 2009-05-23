/*-----------------------------------------------------------------------------

  Win.cpp

  2006 Shamus Young

-------------------------------------------------------------------------------

  Create the main window and make it go.

-----------------------------------------------------------------------------*/

#define MOUSE_MOVEMENT          0.5f

#ifdef WINDOWS
#include <windows.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <GL/glx.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#endif

#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef WINDOWS
#include <scrnsave.h>
#endif

#include "Camera.h"
#include "Car.h"
#include "Entity.h"
#include "glTypes.h"
#include "Ini.h"
#include "Macro.h"
#include "Random.h"
#include "Render.h"
#include "Texture.h"
#include "Win.h"
#include "World.h"
#include "Visible.h"

#ifdef WINDOWS
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "glu32.lib")
#if SCREENSAVER
#pragma comment (lib, "scrnsave.lib")
#endif	
#endif


#ifdef WINDOWS
static HWND         hwnd;
static HINSTANCE    module;
#else
struct POINT {
  int x;
  int y;
};

static Display     *dpy;
static XVisualInfo *vis;
static Window      wnd;
static Atom        del_atom;

static Cursor      blank;
static Atom        wm_state;
static Atom        fullscreen;

#define blank_width 8
#define blank_height 8
#define blank_x_hot 0
#define blank_y_hot 0
static char blank_bits[] = {
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00
};   // mask is the same bit pattern

static void WinHandleEvent(XEvent evt);

#endif

static int          width;
static int          height;
static int          half_width;
static int          half_height;
static bool         lmb;
static bool         rmb;
static bool         mouse_forced;
static POINT        select_pos;
static POINT        mouse_pos;
static bool         quit;

#ifdef WINDOWS

static HINSTANCE    instance;

LONG WINAPI ScreenSaverProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#else

Display *WinGetDisplay()
{
  return dpy;
}

XVisualInfo *WinGetVisual()
{
  return vis;
}

Window WinGetWindow()
{
  return wnd;
}

#endif  /* !WINDOWS */

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void CenterCursor ()
{

#ifdef WINDOWS
  int             center_x;
  int             center_y;
  RECT            rect;

  SetCursor (NULL);
  mouse_forced = true;
  GetWindowRect (hwnd, &rect);
  center_x = rect.left + (rect.right - rect.left) / 2;
  center_y = rect.top + (rect.bottom - rect.top) / 2;
  SetCursorPos (center_x, center_y);
#else
  XWarpPointer(dpy, None, wnd, 0, 0, 0, 0, width / 2, height / 2);
  XFlush(dpy);
#endif

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void MoveCursor (int x, int y)
{

#ifdef WINDOWS
  int             center_x;
  int             center_y;
  RECT            rect;

  SetCursor (NULL);
  mouse_forced = true;
  GetWindowRect (hwnd, &rect);
  center_x = rect.left + x;
  center_y = rect.top + y;
  SetCursorPos (center_x, center_y);
#else
  XWarpPointer(dpy, None, wnd, 0, 0, 0, 0, x, y);
  XFlush(dpy);
#endif

}

/*-----------------------------------------------------------------------------
                                    n o t e
-----------------------------------------------------------------------------*/

#ifdef WINDOWS
void WinPopup (char* message, ...)
{

  va_list  		marker;
  char        buf[1024];

  va_start (marker, message);
  vsprintf (buf, message, marker); 
  va_end (marker);
  MessageBox (NULL, buf, APP_TITLE, MB_ICONSTOP | MB_OK | 
    MB_TASKMODAL);

}
#endif

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

int WinWidth (void)
{

  return width;

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void WinMousePosition (int* x, int* y)
{

  *x = select_pos.x;
  *y = select_pos.y;

}


/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

int WinHeight (void)
{

  return height;

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void WinTerm (void)
{
#ifdef WINDOWS
#if !SCREENAVER
  DestroyWindow (hwnd);
#endif
#else
  XFreeCursor(dpy, blank);
  XDestroyWindow(dpy, wnd);
  XFree(vis);
  XCloseDisplay(dpy);
#endif
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

#ifdef WINDOWS
HWND WinHwnd (void)
{

  return hwnd;

}
#endif


/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void AppQuit ()
{

  quit = true;

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void AppUpdate ()
{

  CameraUpdate ();
  EntityUpdate ();
  WorldUpdate ();
  TextureUpdate ();
  VisibleUpdate ();
  CarUpdate ();
  RenderUpdate ();

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void AppInit (void)
{

  RandomInit (time (NULL));
  CameraInit ();
  RenderInit ();
  TextureInit ();
  WorldInit ();

}


/*-----------------------------------------------------------------------------
                                W i n M a i n
-----------------------------------------------------------------------------*/

void AppTerm (void) 
{

  TextureTerm ();
  WorldTerm ();
  RenderTerm ();
  CameraTerm ();
  WinTerm ();

}

#ifdef WINDOWS
/*-----------------------------------------------------------------------------
                                W i n M a i n
-----------------------------------------------------------------------------*/
#if !SCREENSAVER

int PASCAL WinMain (HINSTANCE instance_in, HINSTANCE previous_instance,
  LPSTR command_line, int show_style)
{

 	MSG		  msg;

  instance = instance_in;
  WinInit ();
  AppInit ();
  while (!quit) {
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))	{
			if (msg.message == WM_QUIT)	
				quit = true;
			else {
				TranslateMessage(&msg);			
				DispatchMessage(&msg);			
			}
    } else 
      AppUpdate ();

  }
  AppTerm ();
  return 0;

}

#else

BOOL WINAPI ScreenSaverConfigureDialog (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) { return FALSE; }
BOOL WINAPI RegisterDialogClasses(HANDLE hInst) { return TRUE; }

#endif

LONG WINAPI ScreenSaverProc(HWND hwnd_in,UINT message,WPARAM wparam,LPARAM lparam)
{

  RECT            r;
  int             key;
  float           delta_x, delta_y;
  POINT           p;

  // Handles screen saver messages
  switch(message)
  {
  case WM_SIZE:
    width = LOWORD(lparam);  // width of client area 
    height = HIWORD(lparam); // height of client area 
    if (wparam == SIZE_MAXIMIZED) {
      IniIntSet ("WindowMaximized", 1);
    } else {
      IniIntSet ("WindowWidth", width);
      IniIntSet ("WindowHeight", height);
      IniIntSet ("WindowMaximized", 0);
    }
    RenderResize ();
    break;
  case WM_KEYDOWN:
    key = (int) wparam; 
    if (key == 'R')
      WorldReset (); 
    else if (key == 'W')
      RenderWireframeToggle ();
    else if (key == 'E')
      RenderEffectCycle ();
    else if (key == 'L')
      RenderLetterboxToggle ();
    else if (key == 'F')
      RenderFPSToggle ();
    else if (key == 'G')
      RenderFogToggle ();
    else if (key == 'T')
      RenderFlatToggle ();
    else if (key == VK_F1)
      RenderHelpToggle ();
    else if (key == VK_ESCAPE)
      break;
    else if (!SCREENSAVER) {
      //Dev mode keys
      if (key == 'C')
        CameraAutoToggle (); 
      if (key == 'B')
        CameraNextBehavior ();
      if (key == VK_F5)
        CameraReset ();
      if (key == VK_UP)
        CameraMedial (1.0f);
      if (key == VK_DOWN)
        CameraMedial (-1.0f);
      if (key == VK_LEFT)
        CameraLateral (1.0f);
      if (key == VK_RIGHT)
        CameraLateral (-1.0f);
      if (key == VK_PRIOR)
        CameraVertical (1.0f);
      if (key == VK_NEXT)
        CameraVertical (-1.0f);
      if (key == VK_F5)
        CameraReset ();
      return 0;
    } else
      break;
    return 0;
  case WM_MOVE:
    GetClientRect (hwnd, &r);
    height = r.bottom - r.top;
    width = r.right - r.left;
    IniIntSet ("WindowX", r.left);
    IniIntSet ("WindowY", r.top);
    IniIntSet ("WindowWidth", width);
    IniIntSet ("WindowHeight", height);
    half_width = width / 2;
    half_height = height / 2;
    return 0;
  case WM_LBUTTONDOWN:
    lmb = true;
    SetCapture (hwnd);
    break;
  case WM_RBUTTONDOWN:
    rmb = true;
    SetCapture (hwnd);
    break;
  case WM_LBUTTONUP:
    lmb = false;
    if (!rmb) {
      ReleaseCapture ();
      MoveCursor (select_pos.x, select_pos.y);
    }
    break;
  case WM_RBUTTONUP:
    rmb = false;
    if (!lmb) {
      ReleaseCapture ();
      MoveCursor (select_pos.x, select_pos.y);
    }
    break;
  case WM_MOUSEMOVE:
    p.x = LOWORD(lparam);  // horizontal position of cursor 
    p.y = HIWORD(lparam);  // vertical position of cursor 
    if (p.x < 0 || p.x > width)
      break;
    if (p.y < 0 || p.y > height)
      break;
    if (!mouse_forced && !lmb && !rmb) {
      select_pos = p; 
    }
    if (mouse_forced) {
      mouse_forced = false;
    } else if (rmb || lmb) {
      CenterCursor ();
      delta_x = (float)(mouse_pos.x - p.x) * MOUSE_MOVEMENT;
      delta_y = (float)(mouse_pos.y - p.y) * MOUSE_MOVEMENT;
      if (rmb && lmb) {
        GLvector    pos;
        CameraPan (delta_x);
        pos = CameraPosition ();
        pos.y += delta_y;
        CameraPositionSet (pos);
      } else if (rmb) {
        CameraPan (delta_x);
        CameraForward (delta_y);
      } else if (lmb) {
        GLvector    angle;
        angle = CameraAngle ();
        angle.y -= delta_x;
        angle.x += delta_y;
        CameraAngleSet (angle);
      }
    }
    mouse_pos = p;
    break;
  case WM_CREATE:
    hwnd = hwnd_in;
    if (SCREENSAVER)
      AppInit ();
    SetTimer (hwnd, 1, 7, NULL); 
    return 0;
  case WM_TIMER:
    AppUpdate ();
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
#if SCREENSAVER
  return DefScreenSaverProc(hwnd_in,message,wparam,lparam);
#else
  return DefWindowProc (hwnd_in,message,wparam,lparam);   
#endif

}

#else   /* !WINDOWS */

/* I would just use timerfd_*(), but that's sadly not very portable.  It's only
 * fairly recent, too (2.6.25 kernels and newer).  So, the old self-pipe trick. */
int pipe_fds[2];

static void handle_alarm(int signum)
{
  write(pipe_fds[1], "\0", 1);
}

static void SetupEventLoopSupport(int *x11_fd, int *fd_count, struct itimerval *tv)
{
  *x11_fd = ConnectionNumber(dpy);
  pipe(pipe_fds);

  *fd_count = MAX(*x11_fd, pipe_fds[0]);

  tv->it_interval.tv_sec = 0;
  tv->it_interval.tv_usec = 16666;  /* 60 fps max */
  tv->it_value = tv->it_interval;

  signal(SIGALRM, handle_alarm);
}

static void TeardownEventLoopSupport()
{
  /* cancel timer first, so it doesn't deliver any more signals */
  setitimer(ITIMER_REAL, NULL, NULL);

  /* cancel alarm handler second */
  signal(SIGALRM, SIG_DFL);

  close(pipe_fds[0]);
  close(pipe_fds[1]);
}

int main()
{
  XEvent report;
  struct itimerval tv;
  int x11_fd, fd_count, rv;
  fd_set readfds;
  char buf[4096];

  if(!WinInit())
    return 1;

  AppInit();

  SetupEventLoopSupport(&x11_fd, &fd_count, &tv);

  /* Well, apparently if you don't call AppUpdate() fast enough during setup,
   * the program isn't able to properly build the bloom lighting texture (and
   * maybe also other random things).  So, give it a separate loop, I suppose. */
  while(!quit && !EntityReady()) {
    if(XEventsQueued(dpy, QueuedAfterFlush)) {
      while(XEventsQueued(dpy, QueuedAfterReading)) {
        XNextEvent(dpy, &report);
        WinHandleEvent(report);
      };
    }

    AppUpdate();
  }

  /* Quits should still work from the loading screen. */
  if(quit) {
    AppTerm();
    return 0;
  }

  /* This sets up the framerate-cap/cpu-usage-limiter alarm.  General idea is
   * that this alarm fires every so often (see SetupEventLoopSupport() above
   * for the FPS, expressed as the timer's period in microseconds).  When the
   * timer fires, it calls the handle_alarm function above, which writes one
   * byte into the anonymous pipe.  That wakes up select() (on the read end of
   * that anonymous pipe) the next time it gets gets called.  (The signal wakes
   * up select() too, but there are races if we rely on that.)  When select sees
   * that its source was the pipe, it skips the X11 stuff, and only then calls
   * AppUpdate(). */
  setitimer(ITIMER_REAL, &tv, NULL);

  while (!quit) {
    FD_ZERO(&readfds);
    FD_SET(x11_fd, &readfds);
    FD_SET(pipe_fds[0], &readfds);

    XFlush(dpy);

    rv = select(fd_count + 1, &readfds, NULL, NULL, NULL);

    if(rv == -1 && errno == EINTR)
      continue;

    if(rv < 0) {
      int save_errno = errno;

      std::cerr << "select() returned error: " << strerror(save_errno) << std::endl;
      AppTerm();
      return 1;
    }

    if(FD_ISSET(x11_fd, &readfds)) {
      while(XEventsQueued(dpy, QueuedAfterReading)) {
        XNextEvent(dpy, &report);
        WinHandleEvent(report);
      };
    }

    if(FD_ISSET(pipe_fds[0], &readfds)) {
      read(pipe_fds[0], buf, sizeof(buf));

      AppUpdate();
    }
  }

  TeardownEventLoopSupport();

  AppTerm();

  return 0;
}

#define MOUSE_FUDGE 16

static void WinHandleEvent(XEvent evt)
{
  POINT p;
  float delta_x, delta_y;
  int buttons;
  KeySym key;

  switch(evt.type) {
    case MapNotify:
      /* */
      break;
    case ConfigureNotify:  /* size or position changed */
      width = evt.xconfigure.width;
      height = evt.xconfigure.height;

      half_width = width / 2;
      half_height = height / 2;

      IniIntSet("WindowX", evt.xconfigure.x);
      IniIntSet("WindowY", evt.xconfigure.y);
      IniIntSet("WindowWidth", width);
      IniIntSet("WindowHeight", height);

      RenderResize();

      break;
    case KeyPress:
      key = XLookupKeysym(&evt.xkey, 0);

      if (key == XK_r)
        WorldReset(); 
      else if (key == XK_w)
        RenderWireframeToggle ();
      else if (key == XK_e)
        RenderEffectCycle ();
      else if (key == XK_l)
        RenderLetterboxToggle ();
      else if (key == XK_f)
        RenderFPSToggle ();
      else if (key == XK_g)
        RenderFogToggle ();
      else if (key == XK_t)
        RenderFlatToggle ();
      else if (key == XK_F1)
        RenderHelpToggle ();
      else if (key == XK_Escape)
        AppQuit();
      else if (!SCREENSAVER) {
        //Dev mode keys
        if (key == XK_c)
          CameraAutoToggle (); 
        if (key == XK_b)
          CameraNextBehavior ();
        if (key == XK_F5)
          CameraReset ();
        if (key == XK_Up)
          CameraMedial (1.0f);
        if (key == XK_Down)
          CameraMedial (-1.0f);
        if (key == XK_Left)
          CameraLateral (1.0f);
        if (key == XK_Right)
          CameraLateral (-1.0f);
        if (key == XK_Prior)
          CameraVertical (1.0f);
        if (key == XK_Next)
          CameraVertical (-1.0f);
      } else
        break;
    case ButtonPress:
      if(evt.xbutton.button == Button1)
        lmb = true;
      else if(evt.xbutton.button == Button3)
        rmb = true;
      else
        break;

      if((lmb && !rmb) || (!lmb && rmb))
        XGrabPointer(dpy, wnd, False, PointerMotionMask | ButtonMotionMask |
            ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
            None, blank, evt.xbutton.time);
      break;
    case ButtonRelease:
      if(evt.xbutton.button == Button1)
        lmb = false;
      else if(evt.xbutton.button == Button3)
        rmb = false;
      else
        break;

      if(!lmb && !rmb) {
        XUngrabPointer(dpy, evt.xbutton.time);
        MoveCursor(select_pos.x, select_pos.y);

        /* Don't bother going through the contortions we go through below
         * after calling CenterCursor(), since we know that all the mouse
         * buttons have been released.  The code in MotionNotify that
         * manipulates the camera won't run here... */
      }
      break;
    case MotionNotify:
      p.x = evt.xmotion.x;
      p.y = evt.xmotion.y;
      buttons = evt.xmotion.state & (Button1Mask | Button3Mask);

      if (!mouse_forced && !buttons) {
        select_pos = p; 
      } else if (mouse_forced) {
        mouse_forced = false;
      } else /* buttons */ {
        delta_x = (float)(mouse_pos.x - p.x) * MOUSE_MOVEMENT;
        delta_y = (float)(mouse_pos.y - p.y) * MOUSE_MOVEMENT;

        if(buttons == (Button1Mask | Button3Mask)) {
          GLvector pos;

          CameraPan(delta_x);
          pos = CameraPosition();
          pos.y += delta_y;
          CameraPositionSet(pos);
        } else if(buttons == Button3Mask) {
          CameraPan(delta_x);
          CameraForward(delta_y);
        } else if(buttons == Button1Mask) {
          GLvector angle = CameraAngle();

          angle.y -= delta_x;
          angle.x += delta_y;

          CameraAngleSet(angle);
        }

        if((p.x > width - MOUSE_FUDGE) ||
            (p.x < MOUSE_FUDGE) ||
            (p.y > height - MOUSE_FUDGE) ||
            (p.y < MOUSE_FUDGE)) {
          CenterCursor();

          /* now eat MotionNotify events until we get one with a position
           * "close" to what we warped to in CenterCursor()
           * yeah, this means we end up stalling the render pipeline,
           * and if this never happens, we might overflow the self-pipe
           * too.  *shrug*  if only x11 gave us an indication that a
           * given MotionNotify came from an XWarpPointer call... */

          while(1) {
            // might as well reuse evt
            XMaskEvent(dpy, PointerMotionMask, &evt);

            if((evt.xmotion.x > half_width - MOUSE_FUDGE) &&
                (evt.xmotion.x < half_width + MOUSE_FUDGE) &&
                (evt.xmotion.y > half_height - MOUSE_FUDGE) &&
                (evt.xmotion.y < half_height + MOUSE_FUDGE)) {
              /* reset p, so mouse_pos below gets reset, so we don't
               * interpret this giant motion next time through this
               * function... */
              p.x = evt.xmotion.x;
              p.y = evt.xmotion.y;

              break;
            }
          }
        }
      }

      mouse_pos = p;

      break;
    case ClientMessage:
      /* message from another client; here, likely the WM */
      if(static_cast<Atom>(evt.xclient.data.l[0]) == del_atom)
        AppQuit();
      break;
  }
}

static Bool WaitForNotify(Display *dpy, XEvent *event, XPointer arg) {
  return (event->type == MapNotify) && (event->xmap.window == reinterpret_cast<Window>(arg));
}

#endif  /* !WINDOWS */

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

bool WinInit (void)
{

#ifdef WINDOWS

  WNDCLASSEX    wcex;
  int           x, y;
  int           style;
  bool          max;

	wcex.cbSize         = sizeof(WNDCLASSEX); 
	wcex.style			    = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	  = (WNDPROC)ScreenSaverProc;
	wcex.cbClsExtra		  = 0;
	wcex.cbWndExtra		  = 0;
	wcex.hInstance		  = instance;
	wcex.hIcon			    = NULL;
	wcex.hCursor		    = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	  = NULL;
	wcex.lpszClassName	= APP_TITLE;
	wcex.hIconSm		    = NULL;
  if (!RegisterClassEx(&wcex)) {
    WinPopup ("Cannot create window class");
    return false;
  }
  x = IniInt ("WindowX");
  y = IniInt ("WindowY");
  style = WS_TILEDWINDOW;
  style |= WS_MAXIMIZE;
  width = IniInt ("WindowWidth");
  height = IniInt ("WindowHeight");
  width = CLAMP (width, 800, 2048);
  height = CLAMP (height, 600, 2048);
  half_width = width / 2;
  half_height = height / 2;
  max = IniInt ("WindowMaximized") == 1;
  if (!(hwnd = CreateWindowEx (0, APP_TITLE, APP_TITLE, style,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, instance, NULL))) {
    WinPopup ("Cannot create window");
    return false;
  }
  if (max) 
    ShowWindow (hwnd, SW_MAXIMIZE);
  else
    ShowWindow (hwnd, SW_SHOW);
  UpdateWindow (hwnd);
  return true;

#else  /* !WINDOWS */

  int attrs[] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_DEPTH_SIZE, 1, None};
  XSetWindowAttributes swa;
  XEvent event;
  Window root;
  XColor black;
  XTextProperty name;
  int x, y;
  unsigned char str_name[] = APP_TITLE;

  dpy = XOpenDisplay(NULL);  // use $DISPLAY

  if(!dpy) {
    std::cerr << "Could not open display to " << XDisplayName(NULL) << std::endl;
    return false;
  }

  wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
  fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

  root = RootWindow(dpy, DefaultScreen(dpy));

  vis = glXChooseVisual(dpy, DefaultScreen(dpy), attrs);

  if(!vis) {
    std::cerr << "Could not get a GLX visual.\n";
    return false;
  }

  swa.colormap = XCreateColormap(dpy, root, vis->visual, AllocNone);
  swa.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
    KeyPressMask | ButtonMotionMask | PointerMotionMask;

  XParseColor(dpy, swa.colormap, "rgb:0/0/0", &black);

  if(SCREENSAVER) {
    XWindowAttributes rwa;
    XGetWindowAttributes(dpy, root, &rwa);

    swa.override_redirect = True;

    x = y = 0;
    width = rwa.width;
    height = rwa.height;
  } else {
    x = IniInt("WindowX");
    y = IniInt("WindowY");
    width = IniInt("WindowWidth");
    height = IniInt("WindowHeight");
    width = CLAMP(width, 800, 2048);
    height = CLAMP(height, 600, 2048);
  }

  half_width = width / 2;
  half_height = height / 2;

  if(SCREENSAVER) {
    wnd = XCreateWindow(dpy, root, x, y, width, height, 0, vis->depth,
        InputOutput, vis->visual, CWEventMask | CWColormap | 
        CWOverrideRedirect, &swa);

    // For NET_WM compliant window managers.  override_redirect may not be
    // required for them, but I can't test that...
    memset(&event, 0, sizeof(event));
    event.type = ClientMessage;
    event.xclient.window = wnd;
    event.xclient.message_type = wm_state;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1;  // add
    event.xclient.data.l[1] = fullscreen;
    event.xclient.data.l[2] = 0;

    XSendEvent(dpy, root, False, SubstructureNotifyMask, &event);
  } else {
    wnd = XCreateWindow(dpy, root, x, y, width, height, 0, vis->depth,
        InputOutput, vis->visual, CWEventMask | CWColormap, &swa);
  }

  name.encoding = XA_STRING;
  name.format = 8;  // bits per character
  name.nitems = sizeof(APP_TITLE) - 1;
  name.value = str_name;

  XSetWMName(dpy, wnd, &name);

  XMapWindow(dpy, wnd);

  /* wait for it to appear: glXMakeCurrent may require it to be visible. */
  XIfEvent(dpy, &event, WaitForNotify, reinterpret_cast<XPointer>(wnd));

  del_atom = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
  if(del_atom != None)
    XSetWMProtocols(dpy, wnd, &del_atom, 1);

  /* now build the blank cursor we use when grabbing the pointer */
  Pixmap pm = XCreateBitmapFromData(dpy, wnd, blank_bits, blank_width, blank_height);
  blank = XCreatePixmapCursor(dpy, pm, pm, &black, &black, blank_x_hot, blank_y_hot);
  XFreePixmap(dpy, pm);

  return true;

#endif  /* !WINDOWS */

}
