// ChatWrapper_C.cpp
#include "pch.h"
#include "AIWrapper.h"
#define AIWRAPPER_EXPORTS        /* dllexport for this TU */
#include "AIWrapper_C.h"

using namespace MBBSAIWrapper;

/* ------- helpers ------- */
static inline AIWrapper* cpp(AI_HANDLE h)
{
	return reinterpret_cast<AIWrapper*>(h);
}

/* ------- exported functions ------- */
extern "C" {

    AI_API AI_HANDLE AI_Create(const char* chatModel, const char* imgModel, const char* chatPrompt)
    {
        std::string cm = chatModel ? chatModel : "";
        std::string im = imgModel ? imgModel : "";
        std::string cp = chatPrompt ? chatPrompt : "";
        return new AIWrapper(cm, im, cp);
    }

    AI_API void AI_Destroy(AI_HANDLE h)
    {
        delete cpp(h);
    }

    AI_API void AI_StartSession(AI_HANDLE h, int channel, const char* userid)
    {
        std::string uid = userid ? userid : "";
        cpp(h)->StartSession(channel, uid);
    }

    AI_API void AI_EndSession(AI_HANDLE h, int channel)
	{
		cpp(h)->EndSession(channel);
	}

    AI_API void Chat_SetCallback(AI_HANDLE h, C_ChatCallback cb)
    {
        cpp(h)->SetChatCallback(
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

    AI_API void Chat_ClearHistory(AI_HANDLE h, int channel)
    {
        cpp(h)->ClearConversationHistory(channel);
    }

    AI_API void Chat_SendAsync(AI_HANDLE h, int channel, const char* userMsg)
    {
        std::string msg = userMsg ? userMsg : "";
        cpp(h)->ChatAsync(channel, msg);
    }

	AI_API void Image_SetCallback(AI_HANDLE h, C_ImageCallback cb)
	{
		cpp(h)->SetImageCallback(
			[cb](int cid,
				 const std::string& uid,
				 const std::string& img,
				 unsigned long st,
				 unsigned long rt,
                 bool fin,
				 bool err)
			{
				if (cb) cb(cid, uid.c_str(), img.c_str(), st, rt, fin, err);
			});
	}

	AI_API void Image_PromptAsync(AI_HANDLE h, int channel, const char* prompt)
	{
		std::string msg = prompt ? prompt : "";
		cpp(h)->GetImageAsync(channel, msg);
	}
} /* extern "C" */
