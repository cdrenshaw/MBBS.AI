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
        Relay(std::function<void(std::string)>* cb) : _cb(cb) {}

        void OnMessage(Object^, String^ txt)
        {
            if (_cb && (*_cb))
                (*_cb)(marshal_as<std::string>(txt));
        }
    private:
        std::function<void(std::string)>* _cb;
    };

    /*----------- Impl definition -----------*/
    struct ChatWrapper::Impl
    {
        msclr::gcroot<Chat^>                       chat;
        msclr::gcroot<Relay^>                      relay;
        msclr::gcroot<EventHandler<String^>^>      handler;
        std::function<void(std::string)>           callback;
    };

    /*----------- ctor -----------*/
    ChatWrapper::ChatWrapper()
    {
        _impl = new Impl;

        _impl->chat = gcnew Chat();
        _impl->relay = gcnew Relay(&_impl->callback);

        /* 1️⃣ convert gcroot to managed handle before passing to delegate */
        Relay^ relayObj = _impl->relay;

        EventHandler<String^>^ h = gcnew EventHandler<String^>(
            relayObj, &Relay::OnMessage);

        _impl->handler = h;                         // store
        _impl->chat->OnMessageReceived += h;        // hook
    }

    /*----------- dtor -----------*/
    ChatWrapper::~ChatWrapper()
    {
        Chat^ chat = _impl->chat;                // 2️⃣ retrieve handles
        EventHandler<String^>^ h = _impl->handler;

        if (chat != nullptr && h != nullptr)
            chat->OnMessageReceived -= h;           // unhook

        delete _impl;
        _impl = nullptr;
    }

    /*----------- public API -----------*/
    void ChatWrapper::SetCallback(std::function<void(std::string)> cb)
    {
        _impl->callback = std::move(cb);
    }

    void ChatWrapper::InitializeChat(std::string const& model,
        std::string const& system)
    {
        _impl->chat->InitializeChat(gcnew String(model.c_str()),
            gcnew String(system.c_str()));
    }

    void ChatWrapper::ChatAsync(std::string const& user)
    {
        _impl->chat->ChatAsync(gcnew String(user.c_str()));
    }
}
