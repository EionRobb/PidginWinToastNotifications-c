#define COBJMACROS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <shlobj.h>
#include <ole2.h>
#include "win_toast.h"

WINOLEAPI PropVariantClear(PROPVARIANT *pvar);

// PROPERTYKEY definition (matches Windows SDK / MinGW _tagpropertykey)
#if !defined(_PROPERTYKEY_DEFINED_) && !defined(PROPERTYKEY_DEFINED)
#define _PROPERTYKEY_DEFINED_
#define PROPERTYKEY_DEFINED
typedef struct _tagpropertykey {
    GUID fmtid;
    DWORD pid;
} PROPERTYKEY;
#endif

#ifndef REFPROPERTYKEY
#define REFPROPERTYKEY const PROPERTYKEY *
#endif

#ifndef REFPROPVARIANT
#define REFPROPVARIANT const PROPVARIANT *
#endif

#ifndef PropVariantInit
#define PropVariantInit(pvar) memset((pvar), 0, sizeof(PROPVARIANT))
#endif

static const PROPERTYKEY PKEY_AppUserModel_ID = {
    { 0x9f4c2855, 0x9f79, 0x4b39, { 0xa8, 0xd0, 0xe1, 0xd4, 0x2d, 0xe1, 0xd5, 0xf3 } },
    5
};

// GUIDs - defined as static const to avoid collisions across different MinGW / Windows SDK versions
static const GUID s_IID_IPropertyStore = { 0x886d8eeb, 0x8cf2, 0x4446, { 0x8d, 0x02, 0xcd, 0xba, 0x1d, 0xbd, 0xcf, 0x99 } };
static const GUID s_IID_IInspectable = { 0xaf86e2e0, 0xb12d, 0x4c6a, { 0x9c, 0x5f, 0xd7, 0xaa, 0x65, 0x10, 0x1e, 0x90 } };
static const GUID s_IID_IAgileObject = { 0x94ea2b94, 0xe9cc, 0x49e0, { 0xc0, 0xff, 0xee, 0x64, 0xca, 0x8f, 0x5b, 0x90 } };
static const GUID s_IID_IToastNotificationManagerStatics = { 0x50ac103f, 0xd235, 0x4598, { 0xbb, 0xef, 0x98, 0xfe, 0x4d, 0x1a, 0x3a, 0xd4 } };
static const GUID s_IID_IToastNotifier = { 0x75927b93, 0x03f3, 0x41ec, { 0x91, 0xd3, 0x6e, 0x5b, 0xac, 0x1b, 0x38, 0xe7 } };
static const GUID s_IID_IToastNotificationFactory = { 0x04124b20, 0x82c6, 0x4229, { 0xb1, 0x09, 0xfd, 0x9e, 0xd4, 0x66, 0x2b, 0x53 } };
static const GUID s_IID_IToastNotification = { 0x997e2675, 0x059e, 0x4e60, { 0x8b, 0x06, 0x17, 0x60, 0x91, 0x7c, 0x8b, 0x80 } };
static const GUID s_IID_ITypedEventHandler_ToastNotification_IInspectable = { 0xab54de2d, 0x97d9, 0x5528, { 0xb6, 0xad, 0x10, 0x5a, 0xfe, 0x15, 0x65, 0x30 } };
static const GUID s_IID_IXmlDocument = { 0xf7f3a506, 0x1e87, 0x42d6, { 0xbc, 0xfb, 0xb8, 0xc8, 0x09, 0xfa, 0x54, 0x94 } };
static const GUID s_IID_IXmlDocumentIO = { 0x6cd0e74e, 0xee65, 0x4489, { 0x9e, 0xbf, 0xca, 0x43, 0xe8, 0x7b, 0xa6, 0x37 } };

// IPropertyStore definition for MinGW
#ifndef __IPropertyStore_INTERFACE_DEFINED__
#define __IPropertyStore_INTERFACE_DEFINED__
typedef struct IPropertyStore IPropertyStore;
typedef struct IPropertyStoreVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IPropertyStore *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IPropertyStore *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IPropertyStore *This);
    HRESULT (STDMETHODCALLTYPE *GetCount)(IPropertyStore *This, DWORD *cProps);
    HRESULT (STDMETHODCALLTYPE *GetAt)(IPropertyStore *This, DWORD iProp, PROPERTYKEY *pkey);
    HRESULT (STDMETHODCALLTYPE *GetValue)(IPropertyStore *This, REFPROPERTYKEY key, PROPVARIANT *pv);
    HRESULT (STDMETHODCALLTYPE *SetValue)(IPropertyStore *This, REFPROPERTYKEY key, REFPROPVARIANT propvar);
    HRESULT (STDMETHODCALLTYPE *Commit)(IPropertyStore *This);
} IPropertyStoreVtbl;
struct IPropertyStore {
    IPropertyStoreVtbl *lpVtbl;
};
#endif

// WinRT basic types
typedef void *HSTRING;
typedef struct HSTRING_HEADER {
    union {
        void *Reserved1;
        char Reserved2[24];
    } Reserved;
} HSTRING_HEADER;

typedef enum RO_INIT_TYPE {
    RO_INIT_MULTITHREADED = 1
} RO_INIT_TYPE;

typedef struct EventRegistrationToken {
    __int64 value;
} EventRegistrationToken;

// Function pointer types
typedef HRESULT (WINAPI *pfn_RoInitialize)(RO_INIT_TYPE initType);
typedef void    (WINAPI *pfn_RoUninitialize)(void);
typedef HRESULT (WINAPI *pfn_RoGetActivationFactory)(HSTRING activatableClassId, REFIID iid, void **factory);
typedef HRESULT (WINAPI *pfn_RoActivateInstance)(HSTRING activatableClassId, void **instance);
typedef HRESULT (WINAPI *pfn_WindowsCreateStringReference)(PCWSTR sourceString, UINT32 length, HSTRING_HEADER *hstringHeader, HSTRING *string);
typedef HRESULT (WINAPI *pfn_WindowsCreateString)(PCWSTR sourceString, UINT32 length, HSTRING *string);
typedef HRESULT (WINAPI *pfn_WindowsDeleteString)(HSTRING string);
typedef PCWSTR  (WINAPI *pfn_WindowsGetStringRawBuffer)(HSTRING string, UINT32 *length);
typedef HRESULT (WINAPI *pfn_SetCurrentProcessExplicitAppUserModelID)(PCWSTR AppID);

// COM Interface declarations
typedef struct IInspectable IInspectable;
typedef struct IInspectableVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IInspectable *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IInspectable *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IInspectable *This);
    HRESULT (STDMETHODCALLTYPE *GetIids)(IInspectable *This, ULONG *iidCount, IID **iids);
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(IInspectable *This, HSTRING *className);
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(IInspectable *This, int *trustLevel);
} IInspectableVtbl;
struct IInspectable {
    IInspectableVtbl *lpVtbl;
};

typedef struct IXmlDocument IXmlDocument;
typedef struct IXmlDocumentVtbl {
    IInspectableVtbl Base;
} IXmlDocumentVtbl;
struct IXmlDocument {
    IXmlDocumentVtbl *lpVtbl;
};

typedef struct IXmlDocumentIO IXmlDocumentIO;
typedef struct IXmlDocumentIOVtbl {
    IInspectableVtbl Base;
    HRESULT (STDMETHODCALLTYPE *LoadXml)(IXmlDocumentIO *This, HSTRING xml);
} IXmlDocumentIOVtbl;
struct IXmlDocumentIO {
    IXmlDocumentIOVtbl *lpVtbl;
};

typedef struct ITypedEventHandler_ToastNotification_IInspectable ITypedEventHandler_ToastNotification_IInspectable;
typedef struct ITypedEventHandler_ToastNotification_IInspectableVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(ITypedEventHandler_ToastNotification_IInspectable *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(ITypedEventHandler_ToastNotification_IInspectable *This);
    ULONG   (STDMETHODCALLTYPE *Release)(ITypedEventHandler_ToastNotification_IInspectable *This);
    HRESULT (STDMETHODCALLTYPE *Invoke)(ITypedEventHandler_ToastNotification_IInspectable *This, void *sender, IInspectable *args);
} ITypedEventHandler_ToastNotification_IInspectableVtbl;
struct ITypedEventHandler_ToastNotification_IInspectable {
    ITypedEventHandler_ToastNotification_IInspectableVtbl *lpVtbl;
};

typedef struct ToastActivatedHandler {
    ITypedEventHandler_ToastNotification_IInspectableVtbl *lpVtbl;
    LONG refCount;
    WinToastClickCallback clickCallback;
    void *conv;
} ToastActivatedHandler;

typedef struct IToastNotification IToastNotification;
typedef struct IToastNotificationVtbl {
    IInspectableVtbl Base;
    HRESULT (STDMETHODCALLTYPE *get_Content)(IToastNotification *This, IXmlDocument **value);
    HRESULT (STDMETHODCALLTYPE *put_ExpirationTime)(IToastNotification *This, void *value);
    HRESULT (STDMETHODCALLTYPE *get_ExpirationTime)(IToastNotification *This, void **value);
    HRESULT (STDMETHODCALLTYPE *add_Dismissed)(IToastNotification *This, void *handler, EventRegistrationToken *token);
    HRESULT (STDMETHODCALLTYPE *remove_Dismissed)(IToastNotification *This, EventRegistrationToken token);
    HRESULT (STDMETHODCALLTYPE *add_Activated)(IToastNotification *This, ITypedEventHandler_ToastNotification_IInspectable *handler, EventRegistrationToken *token);
    HRESULT (STDMETHODCALLTYPE *remove_Activated)(IToastNotification *This, EventRegistrationToken token);
    HRESULT (STDMETHODCALLTYPE *add_Failed)(IToastNotification *This, void *handler, EventRegistrationToken *token);
    HRESULT (STDMETHODCALLTYPE *remove_Failed)(IToastNotification *This, EventRegistrationToken token);
} IToastNotificationVtbl;
struct IToastNotification {
    IToastNotificationVtbl *lpVtbl;
};

typedef struct IToastNotificationFactory IToastNotificationFactory;
typedef struct IToastNotificationFactoryVtbl {
    IInspectableVtbl Base;
    HRESULT (STDMETHODCALLTYPE *CreateToastNotification)(IToastNotificationFactory *This, IXmlDocument *content, IToastNotification **result);
} IToastNotificationFactoryVtbl;
struct IToastNotificationFactory {
    IToastNotificationFactoryVtbl *lpVtbl;
};

typedef struct IToastNotifier IToastNotifier;
typedef struct IToastNotifierVtbl {
    IInspectableVtbl Base;
    HRESULT (STDMETHODCALLTYPE *Show)(IToastNotifier *This, IToastNotification *notification);
    HRESULT (STDMETHODCALLTYPE *Hide)(IToastNotifier *This, IToastNotification *notification);
    HRESULT (STDMETHODCALLTYPE *get_Setting)(IToastNotifier *This, int *value);
    HRESULT (STDMETHODCALLTYPE *AddToSchedule)(IToastNotifier *This, void *scheduledToast);
    HRESULT (STDMETHODCALLTYPE *RemoveFromSchedule)(IToastNotifier *This, void *scheduledToast);
    HRESULT (STDMETHODCALLTYPE *GetScheduledToastNotifications)(IToastNotifier *This, void **result);
} IToastNotifierVtbl;
struct IToastNotifier {
    IToastNotifierVtbl *lpVtbl;
};

typedef struct IToastNotificationManagerStatics IToastNotificationManagerStatics;
typedef struct IToastNotificationManagerStaticsVtbl {
    IInspectableVtbl Base;
    HRESULT (STDMETHODCALLTYPE *CreateToastNotifier)(IToastNotificationManagerStatics *This, IToastNotifier **result);
    HRESULT (STDMETHODCALLTYPE *CreateToastNotifierWithId)(IToastNotificationManagerStatics *This, HSTRING applicationId, IToastNotifier **result);
    HRESULT (STDMETHODCALLTYPE *GetTemplateContent)(IToastNotificationManagerStatics *This, int type, IXmlDocument **result);
} IToastNotificationManagerStaticsVtbl;
struct IToastNotificationManagerStatics {
    IToastNotificationManagerStaticsVtbl *lpVtbl;
};

// Module state
static HMODULE g_hComBase = NULL;
static HMODULE g_hShell32 = NULL;
static BOOL g_isInitialized = FALSE;
static BOOL g_hasRoInit = FALSE;
static WinToastClickCallback g_clickCallback = NULL;

static pfn_RoInitialize pRoInitialize = NULL;
static pfn_RoUninitialize pRoUninitialize = NULL;
static pfn_RoGetActivationFactory pRoGetActivationFactory = NULL;
static pfn_RoActivateInstance pRoActivateInstance = NULL;
static pfn_WindowsCreateStringReference pWindowsCreateStringReference = NULL;
static pfn_WindowsCreateString pWindowsCreateString = NULL;
static pfn_WindowsDeleteString pWindowsDeleteString = NULL;
static pfn_WindowsGetStringRawBuffer pWindowsGetStringRawBuffer = NULL;
static pfn_SetCurrentProcessExplicitAppUserModelID pSetCurrentProcessExplicitAppUserModelID = NULL;

static const wchar_t *g_appName = L"Pidgin";
static wchar_t g_aumi[128] = L"";

static const wchar_t *win_toast_get_app_id(void) {
    if (g_aumi[0] == L'\0') {
        _snwprintf(g_aumi, sizeof(g_aumi) / sizeof(g_aumi[0]), L"Pidgin.Pidgin.%lu", (unsigned long)GetCurrentProcessId());
    }
    return g_aumi;
}


// Handler implementation
static HRESULT STDMETHODCALLTYPE Handler_QueryInterface(
    ITypedEventHandler_ToastNotification_IInspectable *This,
    REFIID riid,
    void **ppvObject)
{
    if (!ppvObject) return E_POINTER;
    if (IsEqualGUID(riid, &IID_IUnknown) ||
        IsEqualGUID(riid, &s_IID_IAgileObject) ||
        IsEqualGUID(riid, &s_IID_ITypedEventHandler_ToastNotification_IInspectable))
    {
        *ppvObject = This;
        This->lpVtbl->AddRef(This);
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Handler_AddRef(ITypedEventHandler_ToastNotification_IInspectable *This) {
    ToastActivatedHandler *self = (ToastActivatedHandler *)This;
    return InterlockedIncrement(&self->refCount);
}

static ULONG STDMETHODCALLTYPE Handler_Release(ITypedEventHandler_ToastNotification_IInspectable *This) {
    ToastActivatedHandler *self = (ToastActivatedHandler *)This;
    LONG count = InterlockedDecrement(&self->refCount);
    if (count == 0) {
        free(self);
    }
    return count;
}

static HRESULT STDMETHODCALLTYPE Handler_Invoke(
    ITypedEventHandler_ToastNotification_IInspectable *This,
    void *sender,
    IInspectable *args)
{
    ToastActivatedHandler *self = (ToastActivatedHandler *)This;
    if (self->clickCallback) {
        self->clickCallback(self->conv);
    }
    return S_OK;
}

static ITypedEventHandler_ToastNotification_IInspectableVtbl g_HandlerVtbl = {
    Handler_QueryInterface,
    Handler_AddRef,
    Handler_Release,
    Handler_Invoke
};

static ToastActivatedHandler *create_handler(WinToastClickCallback clickCallback, void *conv) {
    ToastActivatedHandler *h = (ToastActivatedHandler *)malloc(sizeof(ToastActivatedHandler));
    if (!h) return NULL;
    h->lpVtbl = &g_HandlerVtbl;
    h->refCount = 1;
    h->clickCallback = clickCallback;
    h->conv = conv;
    return h;
}

static HSTRING create_hstring(const wchar_t *str) {
    HSTRING hs = NULL;
    if (pWindowsCreateString && str) {
        pWindowsCreateString(str, wcslen(str), &hs);
    }
    return hs;
}

static void free_hstring(HSTRING hs) {
    if (hs && pWindowsDeleteString) {
        pWindowsDeleteString(hs);
    }
}

// Ensure Start Menu shortcut with AUMI
static HRESULT ensure_shortcut(const wchar_t *app_name, const wchar_t *aumi) {
    WCHAR appData[MAX_PATH];
    WCHAR shortcutPath[MAX_PATH];
    WCHAR exePath[MAX_PATH];
    DWORD len = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return E_FAIL;

    _snwprintf(shortcutPath, MAX_PATH, L"%s\\Microsoft\\Windows\\Start Menu\\Programs\\%s.lnk", appData, app_name);
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    IShellLinkW *shellLink = NULL;
    HRESULT hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (void **)&shellLink);
    if (FAILED(hr)) return hr;

    IPersistFile *persistFile = NULL;
    hr = shellLink->lpVtbl->QueryInterface(shellLink, &IID_IPersistFile, (void **)&persistFile);
    if (FAILED(hr)) {
        shellLink->lpVtbl->Release(shellLink);
        return hr;
    }

    BOOL need_save = FALSE;
    DWORD attr = GetFileAttributesW(shortcutPath);
    if (attr != INVALID_FILE_ATTRIBUTES) {
        hr = persistFile->lpVtbl->Load(persistFile, shortcutPath, STGM_READWRITE);
        if (SUCCEEDED(hr)) {
            IPropertyStore *propStore = NULL;
            hr = shellLink->lpVtbl->QueryInterface(shellLink, &s_IID_IPropertyStore, (void **)&propStore);
            if (SUCCEEDED(hr)) {
                PROPVARIANT propvar;
                PropVariantInit(&propvar);
                hr = propStore->lpVtbl->GetValue(propStore, &PKEY_AppUserModel_ID, &propvar);
                if (SUCCEEDED(hr)) {
                    BOOL matches = FALSE;
                    if (propvar.vt == VT_LPWSTR && propvar.pwszVal) {
                        if (wcscmp(propvar.pwszVal, aumi) == 0 ||
                            wcscmp(propvar.pwszVal, L"Pidgin.Pidgin") == 0 ||
                            wcsncmp(propvar.pwszVal, L"Pidgin.Pidgin.", 14) == 0) {
                            matches = TRUE;
                        }
                    }
                    if (!matches) {
                        PropVariantClear(&propvar);
                        PROPVARIANT new_propvar;
                        PropVariantInit(&new_propvar);
                        new_propvar.vt = VT_LPWSTR;
                        new_propvar.pwszVal = (LPWSTR)aumi;
                        propStore->lpVtbl->SetValue(propStore, &PKEY_AppUserModel_ID, &new_propvar);
                        propStore->lpVtbl->Commit(propStore);
                        need_save = TRUE;
                    } else {
                        PropVariantClear(&propvar);
                    }
                }
                propStore->lpVtbl->Release(propStore);
            }
        }
    } else {
        shellLink->lpVtbl->SetPath(shellLink, exePath);
        shellLink->lpVtbl->SetArguments(shellLink, L"");
        shellLink->lpVtbl->SetWorkingDirectory(shellLink, exePath);

        IPropertyStore *propStore = NULL;
        hr = shellLink->lpVtbl->QueryInterface(shellLink, &s_IID_IPropertyStore, (void **)&propStore);
        if (SUCCEEDED(hr)) {
            PROPVARIANT propvar;
            PropVariantInit(&propvar);
            propvar.vt = VT_LPWSTR;
            propvar.pwszVal = (LPWSTR)aumi;
            propStore->lpVtbl->SetValue(propStore, &PKEY_AppUserModel_ID, &propvar);
            propStore->lpVtbl->Commit(propStore);
            propStore->lpVtbl->Release(propStore);
        }
        need_save = TRUE;
    }

    if (need_save) {
        persistFile->lpVtbl->Save(persistFile, shortcutPath, TRUE);
    }

    persistFile->lpVtbl->Release(persistFile);
    shellLink->lpVtbl->Release(shellLink);
    return S_OK;
}

// Convert UTF-8 to wide char, stripping HTML and XML-escaping
static wchar_t *clean_and_escape_text(const char *utf8_str, BOOL strip_html) {
    if (!utf8_str) return NULL;

    size_t in_len = strlen(utf8_str);
    char *clean = (char *)malloc(in_len + 1);
    if (!clean) return NULL;

    size_t out_idx = 0;
    size_t i = 0;

    // Optional simple HTML tag & entity stripping
    while (i < in_len) {
        if (strip_html && utf8_str[i] == '<') {
            // Check for <br> or <p> to turn into newline
            if (_strnicmp(&utf8_str[i], "<br>", 4) == 0 || _strnicmp(&utf8_str[i], "<br/>", 5) == 0 || _strnicmp(&utf8_str[i], "<br />", 6) == 0) {
                clean[out_idx++] = '\n';
            } else if (_strnicmp(&utf8_str[i], "<p>", 3) == 0 || _strnicmp(&utf8_str[i], "<p ", 3) == 0) {
                if (out_idx > 0 && clean[out_idx - 1] != '\n') clean[out_idx++] = '\n';
            }
            // Skip until '>'
            size_t tag_end = i;
            while (tag_end < in_len && utf8_str[tag_end] != '>') tag_end++;
            if (tag_end < in_len) {
                i = tag_end + 1;
                continue;
            }
            // If no closing '>', treat '<' as a regular character
        }

        if (strip_html && utf8_str[i] == '&') {
            if (_strnicmp(&utf8_str[i], "&nbsp;", 6) == 0) {
                clean[out_idx++] = ' ';
                i += 6;
                continue;
            } else if (_strnicmp(&utf8_str[i], "&lt;", 4) == 0) {
                clean[out_idx++] = '<';
                i += 4;
                continue;
            } else if (_strnicmp(&utf8_str[i], "&gt;", 4) == 0) {
                clean[out_idx++] = '>';
                i += 4;
                continue;
            } else if (_strnicmp(&utf8_str[i], "&quot;", 6) == 0) {
                clean[out_idx++] = '"';
                i += 6;
                continue;
            } else if (_strnicmp(&utf8_str[i], "&apos;", 6) == 0) {
                clean[out_idx++] = '\'';
                i += 6;
                continue;
            } else if (_strnicmp(&utf8_str[i], "&amp;", 5) == 0) {
                clean[out_idx++] = '&';
                i += 5;
                continue;
            }
        }

        clean[out_idx++] = utf8_str[i++];
    }
    clean[out_idx] = '\0';

    // Convert UTF-8 to UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, clean, -1, NULL, 0);
    if (wlen <= 0) {
        free(clean);
        return NULL;
    }

    wchar_t *wstr = (wchar_t *)malloc(wlen * sizeof(wchar_t));
    if (!wstr) {
        free(clean);
        return NULL;
    }
    MultiByteToWideChar(CP_UTF8, 0, clean, -1, wstr, wlen);
    free(clean);

    // Now XML escape: count needed characters
    size_t extra = 0;
    for (int j = 0; j < wlen - 1; j++) {
        switch (wstr[j]) {
            case L'&': extra += 4; break; // &amp;
            case L'<': extra += 3; break; // &lt;
            case L'>': extra += 3; break; // &gt;
            case L'"': extra += 5; break; // &quot;
            case L'\'': extra += 5; break; // &apos;
            default: break;
        }
    }

    wchar_t *escaped = (wchar_t *)malloc((wlen + extra) * sizeof(wchar_t));
    if (!escaped) {
        free(wstr);
        return NULL;
    }

    size_t e_idx = 0;
    for (int j = 0; j < wlen - 1; j++) {
        switch (wstr[j]) {
            case L'&':
                memcpy(&escaped[e_idx], L"&amp;", 5 * sizeof(wchar_t));
                e_idx += 5;
                break;
            case L'<':
                memcpy(&escaped[e_idx], L"&lt;", 4 * sizeof(wchar_t));
                e_idx += 4;
                break;
            case L'>':
                memcpy(&escaped[e_idx], L"&gt;", 4 * sizeof(wchar_t));
                e_idx += 4;
                break;
            case L'"':
                memcpy(&escaped[e_idx], L"&quot;", 6 * sizeof(wchar_t));
                e_idx += 6;
                break;
            case L'\'':
                memcpy(&escaped[e_idx], L"&apos;", 6 * sizeof(wchar_t));
                e_idx += 6;
                break;
            default:
                escaped[e_idx++] = wstr[j];
                break;
        }
    }
    escaped[e_idx] = L'\0';
    free(wstr);

    return escaped;
}

WinToastResult win_toast_init(WinToastClickCallback click_cb) {
    g_clickCallback = click_cb;

    if (g_isInitialized) {
        return WIN_TOAST_SUCCESS;
    }

    g_hComBase = LoadLibraryW(L"combase.dll");
    if (!g_hComBase) {
        return WIN_TOAST_SYSTEM_NOT_SUPPORTED;
    }

    pRoInitialize = (pfn_RoInitialize)GetProcAddress(g_hComBase, "RoInitialize");
    pRoUninitialize = (pfn_RoUninitialize)GetProcAddress(g_hComBase, "RoUninitialize");
    pRoGetActivationFactory = (pfn_RoGetActivationFactory)GetProcAddress(g_hComBase, "RoGetActivationFactory");
    pRoActivateInstance = (pfn_RoActivateInstance)GetProcAddress(g_hComBase, "RoActivateInstance");
    pWindowsCreateStringReference = (pfn_WindowsCreateStringReference)GetProcAddress(g_hComBase, "WindowsCreateStringReference");
    pWindowsCreateString = (pfn_WindowsCreateString)GetProcAddress(g_hComBase, "WindowsCreateString");
    pWindowsDeleteString = (pfn_WindowsDeleteString)GetProcAddress(g_hComBase, "WindowsDeleteString");
    pWindowsGetStringRawBuffer = (pfn_WindowsGetStringRawBuffer)GetProcAddress(g_hComBase, "WindowsGetStringRawBuffer");

    if (!pRoGetActivationFactory || !pRoActivateInstance || !pWindowsCreateString || !pWindowsDeleteString) {
        FreeLibrary(g_hComBase);
        g_hComBase = NULL;
        return WIN_TOAST_SYSTEM_NOT_SUPPORTED;
    }

    HRESULT hr = S_OK;
    if (pRoInitialize) {
        hr = pRoInitialize(RO_INIT_MULTITHREADED);
        if (SUCCEEDED(hr)) {
            g_hasRoInit = TRUE;
        }
    }

    g_hShell32 = LoadLibraryW(L"shell32.dll");
    if (g_hShell32) {
        pSetCurrentProcessExplicitAppUserModelID = (pfn_SetCurrentProcessExplicitAppUserModelID)GetProcAddress(g_hShell32, "SetCurrentProcessExplicitAppUserModelID");
        if (pSetCurrentProcessExplicitAppUserModelID) {
            pSetCurrentProcessExplicitAppUserModelID(win_toast_get_app_id());
        }
    }

    ensure_shortcut(g_appName, win_toast_get_app_id());

    g_isInitialized = TRUE;
    return WIN_TOAST_SUCCESS;
}

WinToastResult win_toast_show_message(
    const char *sender,
    const char *message,
    const char *image_path,
    const char *protocol_name,
    void *conv)
{
    if (!g_isInitialized) {
        return WIN_TOAST_NOT_INITIALIZED;
    }
    if (!sender) {
        return WIN_TOAST_INVALID_PARAMS;
    }

    wchar_t *wSender = clean_and_escape_text(sender, FALSE);
    wchar_t *wMessage = message ? clean_and_escape_text(message, TRUE) : NULL;
    wchar_t *wProtocol = protocol_name ? clean_and_escape_text(protocol_name, FALSE) : NULL;

    wchar_t wImagePath[MAX_PATH] = { 0 };
    if (image_path && image_path[0] != '\0') {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, image_path, -1, NULL, 0);
        if (wlen > 0 && wlen < MAX_PATH - 10) {
            wchar_t rawPath[MAX_PATH];
            MultiByteToWideChar(CP_UTF8, 0, image_path, -1, rawPath, MAX_PATH);
            // Replace backslashes with forward slashes
            for (int k = 0; rawPath[k] != L'\0'; k++) {
                if (rawPath[k] == L'\\') rawPath[k] = L'/';
            }
            if (wcsncmp(rawPath, L"file://", 7) == 0) {
                wcsncpy(wImagePath, rawPath, MAX_PATH - 1);
            } else {
                _snwprintf(wImagePath, MAX_PATH, L"file:///%s", rawPath);
            }
        }
    }

    // Build Toast XML dynamically to avoid buffer truncation or overflow
    size_t senderLen = wSender ? wcslen(wSender) : 0;
    size_t msgLen = wMessage ? wcslen(wMessage) : 0;
    size_t protoLen = wProtocol ? wcslen(wProtocol) : 0;
    size_t imgLen = wcslen(wImagePath);
    size_t xmlCapacity = senderLen + msgLen + protoLen + imgLen + 512;

    wchar_t *xmlBuffer = (wchar_t *)malloc(xmlCapacity * sizeof(wchar_t));
    if (!xmlBuffer) {
        if (wSender) free(wSender);
        if (wMessage) free(wMessage);
        if (wProtocol) free(wProtocol);
        return WIN_TOAST_NOT_DISPLAYED;
    }

    int pos = _snwprintf(xmlBuffer, xmlCapacity,
        L"<toast><visual><binding template=\"ToastGeneric\">"
        L"<text>%ls</text>"
        L"<text>%ls</text>",
        wSender ? wSender : L"",
        wMessage ? wMessage : L"");

    if (wImagePath[0] != L'\0' && pos > 0 && (size_t)pos < xmlCapacity) {
        pos += _snwprintf(xmlBuffer + pos, xmlCapacity - pos,
            L"<image placement=\"appLogoOverride\" src=\"%ls\"/>", wImagePath);
    }

    if (wProtocol && wProtocol[0] != L'\0' && pos > 0 && (size_t)pos < xmlCapacity) {
        pos += _snwprintf(xmlBuffer + pos, xmlCapacity - pos,
            L"<text placement=\"attribution\">%ls</text>", wProtocol);
    }

    if (pos > 0 && (size_t)pos < xmlCapacity) {
        _snwprintf(xmlBuffer + pos, xmlCapacity - pos,
            L"</binding></visual></toast>");
    }

    if (wSender) free(wSender);
    if (wMessage) free(wMessage);
    if (wProtocol) free(wProtocol);

    // 1. Create XmlDocument
    HSTRING hsXmlDocClass = create_hstring(L"Windows.Data.Xml.Dom.XmlDocument");
    IInspectable *pXmlInsp = NULL;
    HRESULT hr = pRoActivateInstance(hsXmlDocClass, (void **)&pXmlInsp);
    free_hstring(hsXmlDocClass);
    if (FAILED(hr) || !pXmlInsp) {
        free(xmlBuffer);
        return WIN_TOAST_NOT_DISPLAYED;
    }

    IXmlDocumentIO *pXmlIO = NULL;
    hr = pXmlInsp->lpVtbl->QueryInterface(pXmlInsp, &s_IID_IXmlDocumentIO, (void **)&pXmlIO);
    if (FAILED(hr) || !pXmlIO) {
        free(xmlBuffer);
        pXmlInsp->lpVtbl->Release(pXmlInsp);
        return WIN_TOAST_NOT_DISPLAYED;
    }

    HSTRING hsXml = create_hstring(xmlBuffer);
    free(xmlBuffer);
    hr = pXmlIO->lpVtbl->LoadXml(pXmlIO, hsXml);
    free_hstring(hsXml);
    if (FAILED(hr)) {
        pXmlIO->lpVtbl->Base.Release((IInspectable *)pXmlIO);
        pXmlInsp->lpVtbl->Release(pXmlInsp);
        return WIN_TOAST_NOT_DISPLAYED;
    }

    IXmlDocument *pXmlDoc = NULL;
    hr = pXmlInsp->lpVtbl->QueryInterface(pXmlInsp, &s_IID_IXmlDocument, (void **)&pXmlDoc);
    if (FAILED(hr) || !pXmlDoc) {
        pXmlIO->lpVtbl->Base.Release((IInspectable *)pXmlIO);
        pXmlInsp->lpVtbl->Release(pXmlInsp);
        return WIN_TOAST_NOT_DISPLAYED;
    }

    // 2. Create ToastNotificationFactory & ToastNotification
    HSTRING hsToastClass = create_hstring(L"Windows.UI.Notifications.ToastNotification");
    IToastNotificationFactory *pToastFactory = NULL;
    hr = pRoGetActivationFactory(hsToastClass, &s_IID_IToastNotificationFactory, (void **)&pToastFactory);
    free_hstring(hsToastClass);
    if (FAILED(hr) || !pToastFactory) {
        pXmlDoc->lpVtbl->Base.Release((IInspectable *)pXmlDoc);
        pXmlIO->lpVtbl->Base.Release((IInspectable *)pXmlIO);
        pXmlInsp->lpVtbl->Release(pXmlInsp);
        return WIN_TOAST_NOT_DISPLAYED;
    }

    IToastNotification *pToast = NULL;
    hr = pToastFactory->lpVtbl->CreateToastNotification(pToastFactory, pXmlDoc, &pToast);
    pToastFactory->lpVtbl->Base.Release((IInspectable *)pToastFactory);
    pXmlDoc->lpVtbl->Base.Release((IInspectable *)pXmlDoc);
    pXmlIO->lpVtbl->Base.Release((IInspectable *)pXmlIO);
    pXmlInsp->lpVtbl->Release(pXmlInsp);

    if (FAILED(hr) || !pToast) {
        return WIN_TOAST_NOT_DISPLAYED;
    }

    // Add activated callback
    if (g_clickCallback) {
        ToastActivatedHandler *handler = create_handler(g_clickCallback, conv);
        if (handler) {
            EventRegistrationToken token = { 0 };
            pToast->lpVtbl->add_Activated(pToast, (ITypedEventHandler_ToastNotification_IInspectable *)handler, &token);
            handler->lpVtbl->Release((ITypedEventHandler_ToastNotification_IInspectable *)handler);
        }
    }

    // 3. Create ToastNotifier & Show
    HSTRING hsManagerClass = create_hstring(L"Windows.UI.Notifications.ToastNotificationManager");
    IToastNotificationManagerStatics *pManager = NULL;
    hr = pRoGetActivationFactory(hsManagerClass, &s_IID_IToastNotificationManagerStatics, (void **)&pManager);
    free_hstring(hsManagerClass);
    if (FAILED(hr) || !pManager) {
        pToast->lpVtbl->Base.Release((IInspectable *)pToast);
        return WIN_TOAST_NOT_DISPLAYED;
    }

    HSTRING hsAppId = create_hstring(win_toast_get_app_id());
    IToastNotifier *pNotifier = NULL;
    hr = pManager->lpVtbl->CreateToastNotifierWithId(pManager, hsAppId, &pNotifier);
    free_hstring(hsAppId);
    pManager->lpVtbl->Base.Release((IInspectable *)pManager);

    if (FAILED(hr) || !pNotifier) {
        pToast->lpVtbl->Base.Release((IInspectable *)pToast);
        return WIN_TOAST_NOT_DISPLAYED;
    }

    hr = pNotifier->lpVtbl->Show(pNotifier, pToast);

    pNotifier->lpVtbl->Base.Release((IInspectable *)pNotifier);
    pToast->lpVtbl->Base.Release((IInspectable *)pToast);

    return SUCCEEDED(hr) ? WIN_TOAST_SUCCESS : WIN_TOAST_NOT_DISPLAYED;
}

void win_toast_uninit(void) {
    if (g_hasRoInit && pRoUninitialize) {
        pRoUninitialize();
        g_hasRoInit = FALSE;
    }
    if (g_hShell32) {
        FreeLibrary(g_hShell32);
        g_hShell32 = NULL;
    }
    if (g_hComBase) {
        FreeLibrary(g_hComBase);
        g_hComBase = NULL;
    }
    g_isInitialized = FALSE;
    g_clickCallback = NULL;
    g_aumi[0] = L'\0';
}
