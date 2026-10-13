#ifndef WIN_TOAST_H
#define WIN_TOAST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WIN_TOAST_SUCCESS = 0,
    WIN_TOAST_NOT_INITIALIZED = 1,
    WIN_TOAST_SYSTEM_NOT_SUPPORTED = 2,
    WIN_TOAST_SHELL_LINK_NOT_CREATED = 3,
    WIN_TOAST_INVALID_AUMI = 4,
    WIN_TOAST_INVALID_PARAMS = 5,
    WIN_TOAST_NOT_DISPLAYED = 6,
    WIN_TOAST_UNKNOWN_ERROR = 7
} WinToastResult;

typedef void (*WinToastClickCallback)(void *conv);

WinToastResult win_toast_init(WinToastClickCallback click_cb);
WinToastResult win_toast_show_message(
    const char *sender,
    const char *message,
    const char *image_path,
    const char *protocol_name,
    void *conv
);
void win_toast_uninit(void);

#ifdef __cplusplus
}
#endif

#endif /* WIN_TOAST_H */
