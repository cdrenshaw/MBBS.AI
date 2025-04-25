/* ChatWrapper_C.h  –– to ship to C programs  */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CHATWRAPPER_EXPORTS        /* defined while building the DLL */
#   define CHAT_API __declspec(dllexport)
#else
#   define CHAT_API __declspec(dllimport)
#endif

    typedef void* CHAT_HANDLE;        /* opaque pointer to C++ object */

    typedef void(__cdecl* C_ChatCallback)(
        int           channelId,
		const char*   userId,
        const char*   utf8Text,
		unsigned long stokens,
		unsigned long rtokens,
        int           isFinal,
        int           isError);

    /* life‑cycle */
    CHAT_API CHAT_HANDLE Chat_Create(const char* model, const char* prompt);
    CHAT_API void        Chat_Destroy(CHAT_HANDLE);

    /* configuration */
    CHAT_API void Chat_SetCallback(CHAT_HANDLE, C_ChatCallback);
    CHAT_API void Chat_StartSession(CHAT_HANDLE, int channel, const char* user);
	CHAT_API void Chat_EndSession(CHAT_HANDLE, int channel);

    /* interaction */
    CHAT_API void Chat_ClearHistory(CHAT_HANDLE, int channel);
    CHAT_API void Chat_SendAsync(CHAT_HANDLE, int channel, const char* userMessage);

#ifdef __cplusplus
}  /* extern "C" */
#endif
