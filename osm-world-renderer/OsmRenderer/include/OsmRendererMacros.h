#define CMD_INDEX 0

#define C_CMD_DATABASE_PATH 1
#define C_CMD_STYLESHEET_PATH 2
#define C_CMD_IMG_SIZE 3

#define R_CMD_LATITUDE 1
#define R_CMD_LONGITUDE 2
#define R_CMD_ZOOM 3

#ifdef _WIN32
#define DEFAULT_FONT_FILE "C:\\Windows\\Fonts\\arial.ttf"
#else
#define DEFAULT_FONT_FILE "/usr/share/fonts/TTF/LiberationSans-Regular.ttf"
#endif

#define LOG_PRFX "├ "