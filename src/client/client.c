#include <X11/Xlib.h>

#include "client.h"
#include "finmath/linear_equation.h"

/* Display to display to */
static Display *dis;

/* Screen to display to */
static int screen;

/* Window handle for this window */
static Window win;

/* Graphics context */
static GC gc;

/* hold command buffer */
char command[256] = { 0 };
size_t command_idx = 0;

/* the font to be used */
XFontStruct *font_info;

/* the security that is being display */
struct security *sec = NULL;

static void
_client_init()
{

  /* Open up the display set by the DISPLAY environmental variable */
  dis = XOpenDisplay(NULL);

  /* get the default screen "monitor" of this display */
  screen = DefaultScreen(dis);

  /* get the colors black and white from the screen */
  unsigned long black;
  unsigned long white;
  black = BlackPixel(dis, screen);
  white = WhitePixel(dis, screen);

  /* create the window */
  win = XCreateSimpleWindow(
      dis, RootWindow(dis, screen), 0, 0, 1024, 720, 0, black, black);

  /* set window class and name for compatibility with i3 */
  XClassHint xch;
  xch.res_class = "dialog";
  xch.res_name = "riski";
  XSetClassHint(dis, win, &xch);
  XSetStandardProperties(dis, win, "Crochet", NULL, None, NULL, 0, NULL);

  /* create a graphics context to draw on */
  gc = XCreateGC(dis, win, 0, 0);

  /* set the default foreground and background colors */
  XSetBackground(dis, gc, white);
  XSetForeground(dis, gc, black);

  XSetFillStyle(dis, gc, FillSolid);

  /* setup input for exposure button and keypress */
  XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);

  font_info = XLoadQueryFont(
      dis, "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-iso10646-1");
  XSetFont(dis, gc, font_info->fid);

  /* clear the window and bring it to the top */
  XClearWindow(dis, win);
  XMapRaised(dis, win);
}

static int
_process_keypress(XEvent event)
{

  KeySym key; /* a dealie-bob to handle KeyPress Events */

  if (XLookupString(&event.xkey, &(command[command_idx]), 255, &key, 0) == 1) {
    if (command[command_idx] == 0x00) {
      command[command_idx] = '\x0';
      return false;
    }
    /* use pressed back space */
    if (command[command_idx] == 0x08) {
      command[command_idx] = '\x0';
      command_idx -= 1;
      return false;
    }
    /* user pressed enter */
    else if (command[command_idx] == 0x0d) {
      command[command_idx] = '\x0';
      pprint_info("command: %s", command);
      sec = exchange_get(command);
      if (!sec) {
      }
      command_idx = 0;
      return false;
    } else if (command[command_idx] == 'q') {
      return true;
    } else {
      command_idx += 1;
    }
  }
  return false;
}

static void
_redraw()
{

  static const int pow10[7] = { 1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0,
    1000000.0 };

  // pprint_warn("%s", "redrawing...");

  /* clear the window */
  XClearWindow(dis, win);

  /* set the color to white and a solid fill for drawing */
  XSetForeground(dis, gc, WhitePixel(dis, screen));
  XSetFillStyle(dis, gc, FillSolid);

  XWindowAttributes xwa;
  XGetWindowAttributes(dis, win, &xwa);

  XSetForeground(dis, gc, WhitePixel(dis, screen));
  XSetFillStyle(dis, gc, FillSolid);

  int font_height = font_info->ascent + font_info->descent;

  XSetForeground(dis, gc, WhitePixel(dis, screen));
  XDrawString(
      dis, win, gc, 0, xwa.height - font_info->descent, command, command_idx);

  Colormap colormap = XDefaultColormap(dis, screen);

  XColor upward;
  XColor downward;

  char green[] = "#628E36";
  XParseColor(dis, colormap, green, &upward);
  XAllocColor(dis, colormap, &upward);

  char red[] = "#A60E16";
  XParseColor(dis, colormap, red, &downward);
  XAllocColor(dis, colormap, &downward);

  if (sec) {
    XSetForeground(dis, gc, WhitePixel(dis, screen));
    XDrawString(
        dis, win, gc, 0, font_info->ascent, sec->name, strlen(sec->name));

    char data[256] = { 0 };
    sprintf(data, "| BID: %.*f | ASK: %.*f | TS: %lu", sec->display_precision,
        (double)sec->best_bid / pow10[sec->display_precision],
        sec->display_precision,
        (double)sec->best_ask / pow10[sec->display_precision],
        sec->last_update);

    XDrawString(dis, win, gc,
        (strlen(sec->name) + 1) * font_info->per_char->width, font_info->ascent,
        data, strlen(data));

    int yaxis_offset = font_info->per_char->width * 10;
    XDrawLine(dis, win, gc, 0, font_height, xwa.width, font_height);
    XDrawLine(dis, win, gc, xwa.width - yaxis_offset, font_height,
        xwa.width - yaxis_offset, xwa.height - font_height);

    uint32_t num_candles = (uint32_t)floor((xwa.width - yaxis_offset) / 7.0);
    pprint_info("num_candles: %d", num_candles);

    struct chart *cht = sec->chart;
    uint32_t min_value = 1215752190;
    uint32_t max_value = 0;
    size_t start_idx = 0;

    if (cht->cur_candle_idx >= num_candles) {
      start_idx = cht->cur_candle_idx - num_candles;
    }

    for (uint32_t i = start_idx; i <= cht->cur_candle_idx; ++i) {
      if (cht->candles[i].volume != 0) {
        if (cht->candles[i].high > max_value) {
          max_value = cht->candles[i].high;
        }
        if (cht->candles[i].low < min_value) {
          min_value = cht->candles[i].low;
        }
      }
    }

    /*
    struct linear_equation *pixel_to_price = NULL;
    pixel_to_price = linear_equation_new(font_height + (font_info->ascent*2),
    max_value, xwa.height-font_height-font_info->descent, min_value);
    */

    struct linear_equation *price_to_pixel = NULL;
    price_to_pixel =
        linear_equation_new(max_value, font_height + font_info->ascent,
            min_value, xwa.height - font_height - (font_info->ascent));

    for (uint32_t i = min_value; i <= max_value; i += 1) {
      char level[20] = { 0 };
      sprintf(level, "-%.*f", sec->display_precision,
          (double)i / pow10[sec->display_precision]);
      XDrawString(dis, win, gc,
          xwa.width - yaxis_offset + font_info->per_char->width,
          linear_equation_eval(price_to_pixel, i) + font_info->descent, level,
          strlen(level));
    }

    for (uint32_t i = start_idx + 1; i <= cht->cur_candle_idx; ++i) {
      if (cht->candles[i].high != cht->candles[i].low) {
        XSetForeground(dis, gc, WhitePixel(dis, screen));
        XSetFillStyle(dis, gc, FillSolid);

        XDrawLine(dis, win, gc, ((i - start_idx - 1) * 7) + 2,
            linear_equation_eval(price_to_pixel, cht->candles[i].high),
            ((i - start_idx - 1) * 7) + 2,
            linear_equation_eval(price_to_pixel, cht->candles[i].low) - 1);
      }

      if (cht->candles[i].volume != 0) {
        if (cht->candles[i].open > cht->candles[i].close) {
          XSetForeground(dis, gc, downward.pixel);
          XSetFillStyle(dis, gc, FillSolid);

          int64_t candle_height =
              linear_equation_eval(price_to_pixel, cht->candles[i].close) -
              linear_equation_eval(price_to_pixel, cht->candles[i].open);
          XFillRectangle(dis, win, gc, (i - start_idx - 1) * 7,
              linear_equation_eval(price_to_pixel, cht->candles[i].open), 5,
              candle_height);
        }
        if (cht->candles[i].open < cht->candles[i].close) {
          XSetForeground(dis, gc, upward.pixel);
          XSetFillStyle(dis, gc, FillSolid);

          int64_t candle_height =
              linear_equation_eval(price_to_pixel, cht->candles[i].open) -
              linear_equation_eval(price_to_pixel, cht->candles[i].close);
          XFillRectangle(dis, win, gc, (i - start_idx - 1) * 7,
              linear_equation_eval(price_to_pixel, cht->candles[i].close), 5,
              candle_height);
        }
      }
    }

    linear_equation_free(&price_to_pixel);
  }
}

int
client_start()
{

  _client_init();

  XEvent event; /* the XEvent declaration !!! */

  bool finished_displaying = false;

  /*
   * Loop through each event at a time
   */
  while (!finished_displaying) {

    while (XPending(dis)) {
      /*
       * Block until next event
       */
      XNextEvent(dis, &event);
      switch (event.type) {
      case Expose:
        _redraw();
        break;
      case KeyPress:
        if (_process_keypress(event)) {
          finished_displaying = true;
        }
        _redraw();
        break;
      case ButtonPress:
        printf("You pressed a button at (%i,%i) %d\n", event.xbutton.x,
            event.xbutton.y, event.xbutton.button);
        _redraw();
        break;
      case ConfigureNotify:
        _redraw();
        break;
      default:
        _redraw();
        break;
      }
    }
  }

  XFreeGC(dis, gc);
  XDestroyWindow(dis, win);
  XCloseDisplay(dis);

  return 0;
}

int
client_getdisplay(Display **display)
{
  if (dis) {
    *display = dis;
    return true;
  } else {
    *display = NULL;
    return false;
  }
}

int
client_getwindow(Window *window)
{
  *window = win;
  return true;
}
