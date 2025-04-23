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

    typedef void(__cdecl* ChatCallback)(const char* utf8Chunk);

    /* life‑cycle */
    CHAT_API CHAT_HANDLE Chat_Create(void);
    CHAT_API void        Chat_Destroy(CHAT_HANDLE h);

    /* configuration */
    CHAT_API void Chat_SetCallback(CHAT_HANDLE h, ChatCallback cb);
    CHAT_API void Chat_InitializeChat(CHAT_HANDLE h,
        const char* model,
        const char* systemPrompt);

    /* interaction */
    CHAT_API void Chat_SendAsync(CHAT_HANDLE h, const char* userMessage);

#ifdef __cplusplus
}  /* extern "C" */
#endif
