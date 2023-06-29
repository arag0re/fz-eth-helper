
#include "finik_eth_app.h"

#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <notification/notification_messages.h>

#include "eth_worker.h"

#define TAG "FinikEthApp"

static void finik_eth_app_draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    FinikEthApp* app = ctx;

    canvas_clear(canvas);

    DrawMode mode = app->draw_mode;
    if(mode == DRAW_ONLY_PICTURES || mode == DRAW_ALL)
        canvas_draw_icon(canvas, 0, 29, &I_amperka_ru_logo_128x35px);
    if(mode == DRAW_ONLY_TEXT || mode == DRAW_ALL) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 4, 8, "This is an example app!");
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(
            canvas,
            127,
            15,
            AlignRight,
            AlignTop,
            "Some long long long long \n aligned multiline text");
    }
}

static void finik_eth_app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

FinikEthApp* finik_eth_app_alloc() {
    FinikEthApp* app = malloc(sizeof(FinikEthApp));

    app->view_port = view_port_alloc();
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    view_port_draw_callback_set(app->view_port, finik_eth_app_draw_callback, app);
    view_port_input_callback_set(app->view_port, finik_eth_app_input_callback, app->event_queue);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    //app->eth_worker =

    return app;
}

void finik_eth_app_free(FinikEthApp* app) {
    furi_assert(app);

    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);

    furi_message_queue_free(app->event_queue);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
}

int32_t finik_eth_app(void* p) {
    UNUSED(p);
    FinikEthApp* app = finik_eth_app_alloc();

    InputEvent event;

    while(1) {
        if(furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypePress) {
                if(event.key == InputKeyBack)
                    break;
                else if(event.key == InputKeyUp) {
                    FURI_LOG_I(TAG, "example_led_sequence");
                    notification_message(app->notifications, &example_led_sequence);
                } else if(event.key == InputKeyDown) {
                    FURI_LOG_I(TAG, "example_vibro_sequence");
                    notification_message(app->notifications, &example_vibro_sequence);
                } else if(event.key == InputKeyOk) {
                    FURI_LOG_I(TAG, "example_sound_sequence");
                    notification_message(app->notifications, &example_sound_sequence);
                }
            } else if(event.type == InputTypeLong) {
                DrawMode mode = app->draw_mode;
                if(event.key == InputKeyLeft)
                    app->draw_mode = (mode - 1 + TOTAL_DRAW_MODES) % TOTAL_DRAW_MODES;
                else if(event.key == InputKeyRight)
                    app->draw_mode = (mode + 1) % TOTAL_DRAW_MODES;

                view_port_update(app->view_port);
            }
        }
    }

    finik_eth_app_free(app);
    return 0;
}
