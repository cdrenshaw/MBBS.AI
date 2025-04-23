// ChatWrapper_C.cpp
#include "pch.h"
#include "ChatWrapper.h"
#define CHATWRAPPER_EXPORTS        /* dllexport for this TU */
#include "ChatWrapper_C.h"

using namespace MBBSAIWrapper;

/* ------- helpers ------- */
static inline ChatWrapper* asCpp(CHAT_HANDLE h)
{
    return reinterpret_cast<ChatWrapper*>(h);
}

/* ------- exported functions ------- */
extern "C" {

    CHAT_API CHAT_HANDLE Chat_Create(void)
    {
        return new ChatWrapper();
    }

    CHAT_API void Chat_Destroy(CHAT_HANDLE h)
    {
        delete asCpp(h);
    }

    CHAT_API void Chat_SetCallback(CHAT_HANDLE h, ChatCallback cb)
    {
        asCpp(h)->SetCallback(
            [cb](std::string s)
            {
                if (cb) cb(s.c_str());
            });
    }

    CHAT_API void Chat_InitializeChat(CHAT_HANDLE h,
        const char* model,
        const char* systemPrompt)
    {
        asCpp(h)->InitializeChat(model, systemPrompt);
    }

    CHAT_API void Chat_SendAsync(CHAT_HANDLE h, const char* userMessage)
    {
        asCpp(h)->ChatAsync(userMessage);
    }

} /* extern "C" */
