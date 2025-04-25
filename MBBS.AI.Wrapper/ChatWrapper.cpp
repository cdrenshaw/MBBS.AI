// ChatWrapper.cpp
#include "pch.h"
#include "ChatWrapper.h"

#include <msclr/gcroot.h>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace MBBS::AI::Wrapper;
using msclr::interop::marshal_as;

namespace MBBSAIWrapper
{
    /*----------- managed relay ------------*/
    private ref class Relay
    {
    internal:
        Relay(ChatCallback* cb) : _cb(cb) {}

        void OnManagedMessage(Object^, MBBS::AI::OpenAI::ChatEventArgs^ e)
        {
            if (!_cb || !(*_cb)) return;

			std::string uid = marshal_as<std::string>(e->UserId);
            std::string txt = marshal_as<std::string>(e->Message);
            (*_cb)(e->ChannelId, uid, txt, 
                static_cast<unsigned long>(e->SendTokens), 
                static_cast<unsigned long>(e->RecvTokens), 
                e->IsFinal, e->IsError);
        }
    private:
        ChatCallback* _cb;
    };

    /*----------- Impl definition -----------*/
    struct ChatWrapper::Impl
    {
        msclr::gcroot<Chat^> chat;
        msclr::gcroot<Relay^> relay;
        msclr::gcroot<EventHandler<MBBS::AI::OpenAI::ChatEventArgs^>^> handler;
        ChatCallback cb;
    };

    /*----------- ctor -----------*/
    ChatWrapper::ChatWrapper(const std::string& model, const std::string& prompt)
    {
        _impl = new Impl;

        _impl->chat = gcnew Chat(gcnew String(model.c_str()), gcnew String(prompt.c_str()));
        _impl->relay = gcnew Relay(&_impl->cb);

        auto h = gcnew EventHandler<MBBS::AI::OpenAI::ChatEventArgs^>(
            _impl->relay, &Relay::OnManagedMessage);

        _impl->handler = h;                         // store
        _impl->chat->OnMessageReceived += h;        // hook
    }

    /*----------- dtor -----------*/
    ChatWrapper::~ChatWrapper()
    {
        Chat^ chat = static_cast<Chat^>(_impl->chat);
        EventHandler<MBBS::AI::OpenAI::ChatEventArgs^>^ h =
            static_cast<EventHandler<MBBS::AI::OpenAI::ChatEventArgs^>^>(_impl->handler);

        if (chat != nullptr && h != nullptr)
            chat->OnMessageReceived -= h;
        
        delete _impl;
        _impl = nullptr;
    }

    /*----------- public API -----------*/
    void ChatWrapper::SetCallback(ChatCallback cb)
    {
        _impl->cb = std::move(cb);
    }

    void ChatWrapper::StartChatSession(int channel,
        std::string& userid)
    {
        _impl->chat->StartChatSession(channel, gcnew String(userid.c_str()));
    }

    void ChatWrapper::EndChatSession(int channel)
    {
		_impl->chat->EndChatSession(channel);
    }

    void ChatWrapper::ClearConversationHistory(int channel) 
    { 
        _impl->chat->ClearConversationHistory(channel); 
    }

    void ChatWrapper::ChatAsync(int channel, std::string const& msg)
    {
        _impl->chat->ChatAsync(channel, gcnew String(msg.c_str()));
    }
}
