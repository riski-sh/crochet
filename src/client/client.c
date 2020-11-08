#include <X11/Xlib.h>

#include "client.h"

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

/* the double buffer */
Pixmap double_buffer = 0;

/* last recorded window size */
int width = 0;
int height = 0;

/* colors */
XColor upward;
XColor downward;
XColor background;
XColor foreground;

/* mutex lock the draw functions for single threaded drawing */
pthread_mutex_t draw_mutex;

bool finished_displaying = false;

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
  XSetStandardProperties(dis, win, "Feed", NULL, None, NULL, 0, NULL);

  /* create a graphics context to draw on */
  gc = XCreateGC(dis, win, 0, 0);

  /* set the default foreground and background colors */
  XSetBackground(dis, gc, white);
  XSetForeground(dis, gc, black);

  XSetFillStyle(dis, gc, FillSolid);

  /* setup input for exposure button and keypress */
  XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);

  font_info = XLoadQueryFont(dis, "fixed");
  XSetFont(dis, gc, font_info->fid);

  Colormap colormap = XDefaultColormap(dis, screen);

  char green[] = "#628E36";
  XParseColor(dis, colormap, green, &upward);
  XAllocColor(dis, colormap, &upward);

  char red[] = "#f44336";
  XParseColor(dis, colormap, red, &downward);
  XAllocColor(dis, colormap, &downward);

  char dark[] = "#1F1B24";
  XParseColor(dis, colormap, dark, &background);
  XAllocColor(dis, colormap, &background);

  char fg[] = "#F5DEB3";
  XParseColor(dis, colormap, fg, &foreground);
  XAllocColor(dis, colormap, &foreground);

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
  pthread_mutex_lock(&draw_mutex);
  static const int pow10[7] = { 1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0,
    1000000.0 };

  XWindowAttributes xwa;
  XGetWindowAttributes(dis, win, &xwa);

  if (xwa.width < 480 || xwa.height < 240) {
    pthread_mutex_unlock(&draw_mutex);
    return;
  }

  if (xwa.width != width || xwa.height != height) {
    if (double_buffer == 0) {
      double_buffer = XCreatePixmap(dis, win, xwa.width, xwa.height, xwa.depth);
    } else {
      XFreePixmap(dis, double_buffer);
      double_buffer = XCreatePixmap(dis, win, xwa.width, xwa.height, xwa.depth);
    }
  }

  width = xwa.width;
  height = xwa.height;

  /* set the color to white and a solid fill for drawing */
  XSetForeground(dis, gc, background.pixel);
  XSetFillStyle(dis, gc, FillSolid);
  XFillRectangle(dis, double_buffer, gc, 0, 0, xwa.width, xwa.height);

  /* set the color to white and a solid fill for drawing */
  XSetForeground(dis, gc, foreground.pixel);
  XSetFillStyle(dis, gc, FillSolid);

  int font_height = font_info->ascent + font_info->descent;

  XSetForeground(dis, gc, foreground.pixel);
  XDrawString(dis, double_buffer, gc, 0, xwa.height - font_info->descent,
      command, command_idx);

  if (sec) {
    XSetForeground(dis, gc, foreground.pixel);
    XDrawString(dis, double_buffer, gc, 0, font_info->ascent, sec->name,
        strlen(sec->name));

    char data[256] = { 0 };
    sprintf(data, "| BID: %.*f | ASK: %.*f | TS: %lu", sec->display_precision,
        (double)sec->best_bid / pow10[sec->display_precision],
        sec->display_precision,
        (double)sec->best_ask / pow10[sec->display_precision],
        sec->last_update);

    XDrawString(dis, double_buffer, gc,
        (strlen(sec->name) + 1) * font_info->per_char->width, font_info->ascent,
        data, strlen(data));

    int yaxis_offset = font_info->per_char->width * 10;
    XDrawLine(dis, double_buffer, gc, 0, font_height, xwa.width, font_height);
    XDrawLine(dis, double_buffer, gc, xwa.width - yaxis_offset, font_height,
        xwa.width - yaxis_offset, xwa.height - font_height);

    uint32_t num_candles = (uint32_t)floor(
        (xwa.width - yaxis_offset) / font_info->per_char->width);

    struct chart *cht = sec->chart;
    uint32_t min_value = 1215752190;
    uint32_t max_value = 0;
    size_t start_idx = 0;

    /* determine number of candles based on drawing width */
    if (cht->cur_candle_idx >= num_candles) {
      start_idx = cht->cur_candle_idx - num_candles;
    }

    /* find min and max of viewable candles */
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

    uint32_t spread = sec->best_ask - sec->best_bid;
    if (spread != 0) {
      max_value += (spread - (max_value % spread));
      min_value -= (max_value % spread);
      min_value -= (max_value - min_value) % spread;
    }

    struct linear_equation *price_to_pixel = NULL;
    price_to_pixel =
        linear_equation_new(max_value, font_height + font_info->ascent,
            min_value, xwa.height - font_height - (font_info->ascent));
    struct linear_equation *pixel_to_price = NULL;
    pixel_to_price = linear_equation_new(font_height + font_info->ascent,
        max_value, xwa.height - font_height - (font_info->ascent), min_value);

    int64_t pips = (linear_equation_eval(pixel_to_price, 0)) -
        linear_equation_eval(pixel_to_price, font_height + font_info->ascent);

    /* display at max 25 ticks */
    // uint32_t offset = (max_value - min_value) % (sec->best_ask -
    // sec->best_bid); if (offset < (uint32_t) (font_info->ascent)) {
    //  offset =(uint32_t) (2*font_info->ascent);
    //}

    for (uint32_t i = min_value; i <= max_value; i += pips) {
      char level[20] = { 0 };
      sprintf(level, "-%.*f", sec->display_precision,
          (double)i / pow10[sec->display_precision]);
      XDrawString(dis, double_buffer, gc,
          xwa.width - yaxis_offset + font_info->per_char->width,
          linear_equation_eval(price_to_pixel, i) + font_info->descent, level,
          strlen(level));
    }

    XSetForeground(dis, gc, foreground.pixel);
    XSetFillStyle(dis, gc, FillSolid);
    XFillRectangle(dis, double_buffer, gc, xwa.width - yaxis_offset,
        linear_equation_eval(price_to_pixel, sec->best_bid) -
            font_info->ascent + 3,
        yaxis_offset,
        font_info->per_char->descent + (2 * font_info->per_char->ascent) - 6);

    XSetForeground(dis, gc, foreground.pixel);
    XFillRectangle(dis, double_buffer, gc, xwa.width - yaxis_offset,
        linear_equation_eval(price_to_pixel, sec->best_ask) -
            font_info->ascent + 3,
        yaxis_offset,
        font_info->per_char->descent + (2 * font_info->per_char->ascent) - 6);

    XSetForeground(dis, gc, background.pixel);
    XSetFillStyle(dis, gc, FillSolid);
    char level[20] = { 0 };
    sprintf(level, "- %.*f", sec->display_precision,
        (double)sec->best_ask / pow10[sec->display_precision]);
    XDrawString(dis, double_buffer, gc, xwa.width - yaxis_offset,
        linear_equation_eval(price_to_pixel, sec->best_ask) +
            font_info->descent,
        level, strlen(level));
    sprintf(level, "- %.*f", sec->display_precision,
        (double)sec->best_bid / pow10[sec->display_precision]);
    XSetForeground(dis, gc, background.pixel);
    XSetFillStyle(dis, gc, FillSolid);
    XDrawString(dis, double_buffer, gc, xwa.width - yaxis_offset,
        linear_equation_eval(price_to_pixel, sec->best_bid) +
            font_info->descent,
        level, strlen(level));

    for (uint32_t i = start_idx + 1; i <= cht->cur_candle_idx; ++i) {
      if (cht->candles[i].high != cht->candles[i].low) {
        XSetForeground(dis, gc, foreground.pixel);
        XSetFillStyle(dis, gc, FillSolid);

        XDrawLine(dis, double_buffer, gc,
            ((i - start_idx - 1) * (font_info->per_char->width) +
                (font_info->per_char->width / 2)),
            linear_equation_eval(price_to_pixel, cht->candles[i].high),
            ((i - start_idx - 1) * (font_info->per_char->width) +
                (font_info->per_char->width / 2)),
            linear_equation_eval(price_to_pixel, cht->candles[i].low) - 1);
      }

      if (cht->candles[i].volume != 0) {
        if (cht->candles[i].open > cht->candles[i].close) {
          XSetForeground(dis, gc, downward.pixel);
          XSetFillStyle(dis, gc, FillSolid);

          int64_t candle_height =
              linear_equation_eval(price_to_pixel, cht->candles[i].close) -
              linear_equation_eval(price_to_pixel, cht->candles[i].open);
          XFillRectangle(dis, double_buffer, gc,
              (i - start_idx - 1) * (font_info->per_char->width),
              linear_equation_eval(price_to_pixel, cht->candles[i].open),
              (font_info->per_char->width), candle_height);
        } else if (cht->candles[i].open < cht->candles[i].close) {
          XSetForeground(dis, gc, upward.pixel);
          XSetFillStyle(dis, gc, FillSolid);

          int64_t candle_height =
              linear_equation_eval(price_to_pixel, cht->candles[i].open) -
              linear_equation_eval(price_to_pixel, cht->candles[i].close);
          XFillRectangle(dis, double_buffer, gc,
              (i - start_idx - 1) * (font_info->per_char->width),
              linear_equation_eval(price_to_pixel, cht->candles[i].close),
              (font_info->per_char->width), candle_height);
        } else if (cht->candles[i].open == cht->candles[i].close) {
          XSetForeground(dis, gc, foreground.pixel);
          XSetFillStyle(dis, gc, FillSolid);
          XDrawLine(dis, double_buffer, gc,
              (i - start_idx - 1) * (font_info->per_char->width),
              linear_equation_eval(price_to_pixel, cht->candles[i].close),
              ((i - start_idx - 1) * (font_info->per_char->width)) +
                  (font_info->per_char->width),
              linear_equation_eval(price_to_pixel, cht->candles[i].close));
        }

        struct chart_object *analysis_iter = cht->candles[i].analysis_list;

        int draw_height = 0;

        while (analysis_iter) {

          switch (analysis_iter->object_type) {
          case CHART_OBJECT_TEXT: {
            struct chart_object_t_text *analysis_object = analysis_iter->value;
            XSetForeground(dis, gc, foreground.pixel);
            XSetFillStyle(dis, gc, FillSolid);
            XDrawString(dis, double_buffer, gc,
                (i - start_idx - 1) * (font_info->per_char->width),
                linear_equation_eval(price_to_pixel, cht->candles[i].low) +
                    (font_info->per_char->ascent +
                        font_info->per_char->descent + 5 +
                        (draw_height *
                            (font_info->per_char->ascent +
                                font_info->per_char->descent))),
                &(analysis_object->TEXT), 1);
            draw_height += 1;
            break;
          }
          }

          analysis_iter = analysis_iter->next;
        }
      }
    }

    linear_equation_free(&price_to_pixel);
  }

  XCopyArea(dis, double_buffer, win, gc, 0, 0, xwa.width, xwa.height, 0, 0);
  XFlush(dis);
  pthread_mutex_unlock(&draw_mutex);
}

int
client_start()
{

  /* tell X11 that there are multiple threads to this program */
  XInitThreads();

  /* setup the drawing window, color scheme etc... */
  _client_init();

  /* declare an XEvent */
  XEvent event;

  /*
   * Loop through each event at a time
   */
  while (globals_continue(NULL)) {

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
        break;
      case ConfigureNotify:
        break;
      default:
        break;
      }
    }
  }

  finished_displaying = true;

  pthread_mutex_lock(&draw_mutex);
  XFreeGC(dis, gc);
  XDestroyWindow(dis, win);
  XCloseDisplay(dis);
  pthread_mutex_unlock(&draw_mutex);

  return 0;
}

int
client_getdisplay(Display **display)
{
  if (!finished_displaying && globals_continue(NULL)) {
    if (dis) {
      *display = dis;
      return true;
    } else {
      *display = NULL;
      return false;
    }
  } else {
    return false;
  }
}

int
client_getwindow(Window *window)
{
  if (!finished_displaying && globals_continue(NULL)) {
    *window = win;
    return true;
  } else {
    return false;
  }
}

void
client_redraw()
{
  if (!finished_displaying) {
    _redraw();
  }
}
