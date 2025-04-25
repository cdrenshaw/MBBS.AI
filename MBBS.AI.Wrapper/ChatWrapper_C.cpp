// ChatWrapper_C.cpp
#include "pch.h"
#include "ChatWrapper.h"
#define CHATWRAPPER_EXPORTS        /* dllexport for this TU */
#include "ChatWrapper_C.h"

using namespace MBBSAIWrapper;

/* ------- helpers ------- */
static inline ChatWrapper* cpp(CHAT_HANDLE h)
{
	return reinterpret_cast<ChatWrapper*>(h);
}

/* ------- exported functions ------- */
extern "C" {

    CHAT_API CHAT_HANDLE Chat_Create(const char* model, const char* prompt)
    {
        std::string m = model ? model : "";
        std::string p = prompt ? prompt : "";
        return new ChatWrapper(m, p);
    }

    CHAT_API void Chat_Destroy(CHAT_HANDLE h)
    {
        delete cpp(h);
    }

    CHAT_API void Chat_SetCallback(CHAT_HANDLE h, C_ChatCallback cb)
    {
        cpp(h)->SetCallback(
            [cb](int cid, 
                 const std::string& uid, 
                 const std::string& txt, 
                 unsigned long st, 
                 unsigned long rt, 
                 bool fin, 
                 bool err)
            {
                if (cb) cb(cid, uid.c_str(), txt.c_str(), st, rt, fin, err);
            });
    }

    CHAT_API void Chat_StartSession(CHAT_HANDLE h, int channel, const char* userid)
    {
        std::string uid = userid ? userid : "";
        cpp(h)->StartChatSession(channel, uid);
    }

	CHAT_API void Chat_EndSession(CHAT_HANDLE h, int channel)
	{
		cpp(h)->EndChatSession(channel);
	}

    CHAT_API void Chat_ClearHistory(CHAT_HANDLE h, int channel)
    {
        cpp(h)->ClearConversationHistory(channel);
    }

    CHAT_API void Chat_SendAsync(CHAT_HANDLE h, int channel, const char* userMsg)
    {
        std::string msg = userMsg ? userMsg : "";
        cpp(h)->ChatAsync(channel, msg);
    }

} /* extern "C" */
