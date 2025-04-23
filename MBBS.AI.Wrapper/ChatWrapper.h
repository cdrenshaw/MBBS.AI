// ChatWrapper.h 
#pragma once

#include "Exports.h"
#include <functional>
#include <string>

namespace MBBSAIWrapper
{
    class MBBSAI_API ChatWrapper
    {
        struct Impl;           // forward declaration
        Impl* _impl = nullptr; // native pointer

    public:
        ChatWrapper();
        ~ChatWrapper();

        void SetCallback(std::function<void(std::string)> cb);
        void InitializeChat(std::string const& model,
            std::string const& systemMessage);
        void ChatAsync(std::string const& userMessage);

        ChatWrapper(ChatWrapper const&) = delete;
        ChatWrapper& operator=(ChatWrapper const&) = delete;
    };
}

#ifdef _MSC_VER           // save the host a linker‑setting step
# ifndef MBBSAIWRAPPER_EXPORTS          // emit only for external users
#   pragma comment(lib, "MBBS.AI.Wrapper.lib")
# endif
#endif
