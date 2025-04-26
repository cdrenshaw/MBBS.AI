// ChatWrapper.h 
#pragma once

#include "Exports.h"
#include <functional>
#include <string>

namespace MBBSAIWrapper
{
    using ChatCallback = std::function<void(
        int,                                        /* channelId            */ 
		const std::string&, 					    /* userId               */
		const std::string&,     	                /* text                 */
		unsigned long,  					        /* stokens              */
		unsigned long,  					        /* rtokens              */
		bool isFinal,   				            /* isFinal              */
        bool isError)>;                             /* isError              */

    using ImageCallback = std::function<void(
		int,                                        /* channelId            */ 
		const std::string&, 					    /* userId               */  
		const std::string&,     	                /* imgANSI              */  
		unsigned long,      	                    /* stokens              */
		unsigned long,	  	                        /* rtokens              */
		bool isError,   						    /* isError              */
		const std::string&)>;			            /* errMsg               */

    class MBBSAI_API AIWrapper
    {
        struct Impl;                                // forward declaration
        Impl* _impl = nullptr;                      // native pointer

    public:
        AIWrapper(const std::string& chatModel, const std::string& imgModel, const std::string& chatPrompt);
        ~AIWrapper();

        void StartSession(int channel, std::string& userid);
        void EndSession(int channel);
        void SetChatCallback(ChatCallback);
        void ClearConversationHistory(int channel);
        void ChatAsync(int channel, std::string const& userMessage);
        void SetImageCallback(ImageCallback);
		void GetImageAsync(int channel, std::string const& prompt);

        AIWrapper(const AIWrapper&) = delete;
        AIWrapper& operator=(AIWrapper const&) = delete;
    };
}

#ifdef _MSC_VER                                     // save the host a linker‑setting step
# ifndef MBBSAIWRAPPER_EXPORTS                      // emit only for external users
#   pragma comment(lib, "MBBS.AI.Wrapper.lib")
# endif
#endif
