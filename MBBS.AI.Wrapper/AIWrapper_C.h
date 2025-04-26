/* ChatWrapper_C.h  –– to ship to C programs  */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef AIWRAPPER_EXPORTS        /* defined while building the DLL */
#   define AI_API __declspec(dllexport)
#else
#   define AI_API __declspec(dllimport)
#endif

    typedef void* AI_HANDLE;        /* opaque pointer to C++ object */

    typedef void(__cdecl* C_ChatCallback)(
        int                 channelId,
		const char*         userId,
        const char*         utf8Text,
		unsigned long       stokens,
		unsigned long       rtokens,
        int                 isFinal,
        int                 isError);

    typedef void(__cdecl* C_ImageCallback)(
        int                 channelId,
        const char*         userId,
		const char*         imgANSI,
        unsigned long       stokens,
        unsigned long       rtokens,
        int                 isFinal,
        int                 isError);

    /* life‑cycle */
    AI_API AI_HANDLE AI_Create(const char* chatModel, const char* imgModel, const char* chatPrompt);
    AI_API void AI_Destroy(AI_HANDLE);

    /* configuration */
    AI_API void AI_StartSession(AI_HANDLE, int channel, const char* user);
    AI_API void AI_EndSession(AI_HANDLE, int channel);
    AI_API void Chat_SetCallback(AI_HANDLE, C_ChatCallback);
	AI_API void Image_SetCallback(AI_HANDLE, C_ImageCallback);
    
    /* interaction */
    AI_API void Chat_ClearHistory(AI_HANDLE, int channel);
    AI_API void Chat_SendAsync(AI_HANDLE, int channel, const char* userMessage);
	AI_API void Image_PromptAsync(AI_HANDLE, int channel, const char* prompt);

#ifdef __cplusplus
}  /* extern "C" */
#endif
