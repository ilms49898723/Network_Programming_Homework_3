#ifndef COLORCODES_H_
#define COLORCODES_H_

#ifdef ENABLE_COLOR

#define COLOR_NORMAL          "\x1B[0m"
#define COLOR_RED             "\x1B[31m"
#define COLOR_GREEN           "\x1B[32m"
#define COLOR_YELLOW          "\x1B[33m"
#define COLOR_BLUE            "\x1B[34m"
#define COLOR_MAGENTA         "\x1B[35m"
#define COLOR_CYAN            "\x1B[36m"
#define COLOR_WHITE           "\x1B[37m"
#define COLOR_BRIGHT_RED      "\x1B[1;31m"
#define COLOR_BRIGHT_GREEN    "\x1B[1;32m"
#define COLOR_BRIGHT_YELLOW   "\x1B[1;33m"
#define COLOR_BRIGHT_BLUE     "\x1B[1;34m"
#define COLOR_BRIGHT_MAGENTA  "\x1B[1;35m"
#define COLOR_BRIGHT_CYAN     "\x1B[1;36m"
#define COLOR_BRIGHT_WHITE    "\x1B[1;37m"

#else

#define COLOR_NORMAL          ""
#define COLOR_RED             ""
#define COLOR_GREEN           ""
#define COLOR_YELLOW          ""
#define COLOR_BLUE            ""
#define COLOR_MAGENTA         ""
#define COLOR_CYAN            ""
#define COLOR_WHITE           ""
#define COLOR_BRIGHT_RED      ""
#define COLOR_BRIGHT_GREEN    ""
#define COLOR_BRIGHT_YELLOW   ""
#define COLOR_BRIGHT_BLUE     ""
#define COLOR_BRIGHT_MAGENTA  ""
#define COLOR_BRIGHT_CYAN     ""
#define COLOR_BRIGHT_WHITE    ""

#endif // ENABLE_COLOR

#endif // COLORCODES_H_

