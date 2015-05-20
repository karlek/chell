#define _colors_h

#define BLACK   "\x1b[90m"
#define RED		"\x1b[91m"
#define GREEN   "\x1b[92m"
#define YELLOW  "\x1b[93m"
#define BLUE	"\x1b[94m"
#define MAGENTA "\x1b[95m"
#define CYAN	"\x1b[96m"
#define RESET   "\x1b[0m"


/* The 5 is which color index 0-7; then 0-255m.*/
/* Hex color codes can be calculated with: COLOR = r*6^2 + g*6 + b) + 16.*/
#define SPECIAL "\x1b[38;5;150m"
