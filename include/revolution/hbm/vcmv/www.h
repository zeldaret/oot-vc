#ifndef WWW_H
#define WWW_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern UNKWORD (*WWWSurfaceInit)(u16, u16, u32, UNKWORD, void*);
extern void (*WWWSurfaceNewScreen)();
extern void (*WWWSurfaceDeleteScreen)();
extern void (*WWWSurfaceResize)();
extern void (*WWWSurfaceShutdown)();
extern UNKWORD (*WWWSurfaceSetFlushCallback)(void*, UNKWORD);
extern void (*WWWSurfaceInvalidate)();
extern void (*WWWSurfaceUpdateScreen)(UNKWORD);
extern void (*WWWSurfaceLockArea)();
extern void (*WWWSurfaceUnlockArea)();
extern void (*WWWSurfaceMouseEvt)(UNKWORD, UNKWORD, UNKWORD, UNKWORD, UNKWORD, UNKWORD);
extern void (*WWWSurfaceWheelEvt)(UNKWORD, UNKWORD, UNKWORD, UNKWORD, UNKWORD);
extern void (*WWWSurfaceKeyboardEvt)();
extern void (*WWWSurfaceAddFont)(const char*);
extern UNKWORD (*WWWCreateBrowser)(void*, void*, const char**, const char*);
extern void (*WWWTerminateBrowser)();
extern UNKWORD (*WWWRunSlice)(UNKWORD);
extern UNKWORD (*WWWCreateBrowserWindow)(UNKWORD, void*, UNKWORD);
extern void (*WWWCloseBrowserWindow)(UNKWORD, UNKWORD);
extern void (*WWWSetBrowserWindowTransparent)();
extern void (*WWWGetBrowserWindowRect)();
extern void (*WWWSetBrowserWindowRect)(UNKWORD, void*);
extern void (*WWWRaiseBrowserWindow)(UNKWORD);
extern void (*WWWLowerBrowserWindow)();
extern void (*WWWShowBrowserWindow)(UNKWORD);
extern void (*WWWHideBrowserWindow)();
extern void (*WWWCommitIme)();
extern void (*WWWUpdateIme)();
extern void (*WWWPostUrl)();
extern void (*WWWOpenUrl)(UNKWORD, const char*);
extern void (*WWWGetHistoryCount)();
extern void (*WWWNextPage)(UNKWORD);
extern void (*WWWPrevPage)(UNKWORD);
extern void (*WWWMoveInHistory)();
extern void (*WWWStop)();
extern void (*WWWReload)();
extern void (*WWWReflow)();
extern void (*WWWSearch)();
extern void (*WWWResetSearch)();
extern void (*WWWSetFocus)(UNKWORD);
extern void (*WWWLoseFocus)();
extern void (*WWWHistory)();
extern void (*WWWClearHistory)();
extern void (*WWWGetTrueZoom)();
extern void (*WWWSetTrueZoom)();
extern void (*WWWGetZoom)();
extern void (*WWWSetZoom)();
extern void (*WWWGetSecurityMode)();
extern void (*WWWGetScroll)();
extern void (*WWWSetScroll)();
extern void (*WWWSetRenderingMode)(UNKWORD, UNKWORD);
extern void (*WWWGetRenderingMode)();
extern void (*WWWSetImageMode)(UNKWORD, UNKWORD);
extern void (*WWWCreateCertificateManager)();
extern void (*WWWCloseCertificateManager)();
extern void (*WWWGetNumberOfCertificates)();
extern void (*WWWSetLanguageEncoding)();
extern void (*WWWGetDocumentIcon)();
extern void (*WWWGetDocumentIconUrl)();
extern void (*WWWClearCookies)();
extern void (*WWWGetDocumentSize)();
extern void (*WWWSetIntPref)();
extern void (*WWWGetIntPref)();
extern void (*WWWSetStringPref)();
extern void (*WWWGetStringPref)();
extern void (*WWWCommitPrefs)();
extern void (*WWWSetFocusColors)();
extern void (*WWWSetScrollbarColors)();
extern void (*WWWSetScrollbarSize)();
extern void (*WWWSetWidgetColors)();
extern void (*WWWSetDisabledWidgetColors)();
extern void (*WWWSetButtonWidgetColors)();
extern void (*WWWSetUastringExtension)();
extern void (*WWWMarkNextItemInDirection)();
extern void (*WWWResetNavigation)();
extern void (*WWWClearHighlight)();
extern void (*WWWSetHighlight)();
extern void (*WWWGetActiveLinkType)();
extern void (*WWWGetBrowserAllocationFunctions)(void*, u32, void*, void*, void*);
extern void (*WWWShutdownBrowserAllocationFunctions)();
extern void (*WWWSetAllocationFunctions)(UNKWORD, UNKWORD, UNKWORD, UNKWORD, UNKWORD, UNKWORD, UNKWORD);
extern void (*WWWHTTPCreateHttpLib)();
extern void (*WWWHTTPTerminateHttpLib)();
extern void (*WWWHTTPSessionRunSlice)();
extern void (*WWWHTTPInitSession)();
extern void (*WWWHTTPDeleteSession)();
extern void (*WWWHTTPSetSessionHeader)();
extern void (*WWWHTTPRemoveSessionHeader)();
extern void (*WWWHTTPRemoveAllSessionHeaders)();
extern void (*WWWHTTPCreateRequest)();
extern void (*WWWHTTPDeleteRequest)();
extern void (*WWWHTTPSetRequestHeader)();
extern void (*WWWHTTPGetRequestHeader)();
extern void (*WWWHTTPRemoveRequestHeader)();
extern void (*WWWHTTPRemoveAllRequestHeaders)();
extern void (*WWWHTTPGetResponseHeader)();
extern void (*WWWHTTPGetResponseHeaders)();
extern void (*WWWHTTPIssue)();
extern void (*WWWHTTPNbActiveRequests)();
extern void (*WWWHTTPPostBodyData)();
extern void (*WWWHTTPSetAuthCredentials)();
extern void (*WWWHTTPRemoveAuthCredentials)();
extern void (*WWWHTTPSetProxy)();
extern void (*WWWHTTPRemoveProxy)();
extern void (*WWWHTTPEndLoading)();
extern void (*WWWAddJSPlugin)(const char*, void*, void*, void*, void*);
extern void (*WWWAddNSPlugin)();
extern UNKWORD (*WWWProtocolWrite)(UNKWORD, void*, UNKWORD);
extern UNKWORD (*WWWProtocolSetMimeType)(UNKWORD, const char*);
extern UNKWORD (*WWWProtocolFinished)(UNKWORD);
extern UNKWORD (*WWWProtocolFailed)(UNKWORD);
extern void (*WWWAddProtocol)(const char*);

#ifdef __cplusplus
}
#endif

#endif
