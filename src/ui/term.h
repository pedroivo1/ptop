#ifndef TERM_H
#define TERM_H

#define CSI(seq)            "\033[" seq

// Screen & Buffer
#define TUI_ALT_BUF_ON      CSI("?1049h")
#define TUI_ALT_BUF_OFF     CSI("?1049l")
#define TUI_CLEAR           CSI("2J")
#define TUI_HOME            CSI("H")

// Cursor
#define TUI_CURSOR_HIDE     CSI("?25l")
#define TUI_CURSOR_SHOW     CSI("?25h")

// Mouse Tracking
#define TUI_MOUSE_ON        CSI("?1000h")
#define TUI_MOUSE_OFF       CSI("?1000l")
#define TUI_SGR_MOUSE_ON    CSI("?1006h")
#define TUI_SGR_MOUSE_OFF   CSI("?1006l")

#define TUI_RESET_ATTR      CSI("0m")

// Setup: AltBuf -> HideCursor -> Mouse -> Clear -> Home
#define TUI_INIT_SEQ \
    TUI_ALT_BUF_ON \
    TUI_CURSOR_HIDE \
    TUI_MOUSE_ON \
    TUI_SGR_MOUSE_ON \
    TUI_CLEAR \
    TUI_HOME

// Restore: Reset -> AltBufOff -> ShowCursor -> MouseOff
#define TUI_EXIT_SEQ \
    TUI_RESET_ATTR \
    TUI_ALT_BUF_OFF \
    TUI_CURSOR_SHOW \
    TUI_MOUSE_OFF \
    TUI_SGR_MOUSE_OFF

#endif
