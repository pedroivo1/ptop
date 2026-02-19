#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include <stddef.h>
#include "app/app.h"

void app_update_layout(AppContext ctx[static 1]);
void app_handle_input(AppContext ctx[static 1], int timeout_ms);
void app_update_state(AppContext ctx[static 1]);

ptrdiff_t app_draw(AppContext ctx[static 1], char buf[static 1]);

#endif
