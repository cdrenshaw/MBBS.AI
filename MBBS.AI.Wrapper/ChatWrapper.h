// ChatWrapper.h 
#pragma once

#include "Exports.h"
#include <functional>
#include <string>

namespace MBBSAIWrapper
{
    /*  callback signature:
        channelId  – int 
		userId     – UTF-8  std::string
        text       – UTF-8  std::string
        stokens    - ulong
		rtokens    - ulong
        isFinal    – true when the assistant finished a turn
        isError    – true when the message is an error
    */

    using ChatCallback = std::function<void(
        int, 
        const std::string&, 
        const std::string&, 
        unsigned long, 
        unsigned long, 
        bool isFinal, 
        bool isError)>;

    class MBBSAI_API ChatWrapper
    {
        struct Impl;           // forward declaration
        Impl* _impl = nullptr; // native pointer

    public:
        ChatWrapper(const std::string& model, const std::string& prompt);
        ~ChatWrapper();

        void SetCallback(ChatCallback);
        void StartChatSession(int channel, std::string& userid);
        void EndChatSession(int channel);
        void ClearConversationHistory(int channel);
        void ChatAsync(int channel, std::string const& userMessage);

        ChatWrapper(const ChatWrapper&) = delete;
        ChatWrapper& operator=(ChatWrapper const&) = delete;
    };
}

#ifdef _MSC_VER           // save the host a linker‑setting step
# ifndef MBBSAIWRAPPER_EXPORTS          // emit only for external users
#   pragma comment(lib, "MBBS.AI.Wrapper.lib")
# endif
#endif
