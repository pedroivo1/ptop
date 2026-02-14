#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include "app/app.h"

void app_update_layout(AppContext *ctx);
void app_handle_input(AppContext *ctx, int timeout_ms);
void app_update_state(AppContext *ctx);

int app_draw(AppContext *ctx, char *buffer);

#endif
