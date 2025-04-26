// ChatWrapper.cpp
#include "pch.h"
#include "AIWrapper.h"

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
        Relay(ChatCallback* ccb) : _chatcb(ccb) {}
		Relay(ImageCallback* icb) : _imgcb(icb) {}

        void OnManagedMessage(Object^, MBBS::AI::OpenAI::ChatEventArgs^ e)
        {
            if (!_chatcb || !(*_chatcb)) return;

			std::string uid = marshal_as<std::string>(e->UserId);
            std::string txt = marshal_as<std::string>(e->Message);
            (*_chatcb)(e->ChannelId, uid, txt,
                static_cast<unsigned long>(e->SendTokens), 
                static_cast<unsigned long>(e->RecvTokens), 
                e->IsFinal, e->IsError);
        }

        void OnImageMessage(Object^, MBBS::AI::OpenAI::ImageEventArgs^ e)
        {
            if (!_imgcb || !(*_imgcb)) return;

			std::string uid = marshal_as<std::string>(e->UserId);
			std::string img = marshal_as<std::string>(e->Message);
			(*_imgcb)(e->ChannelId, uid, img,
				static_cast<unsigned long>(e->SendTokens),
				static_cast<unsigned long>(e->RecvTokens),
				e->IsError);
        }
    private:
        ChatCallback* _chatcb;
        ImageCallback* _imgcb;
    };

    /*----------- Impl definition -----------*/
    struct AIWrapper::Impl
    {
        msclr::gcroot<AIService^> ai;
        msclr::gcroot<Relay^> relay;
        msclr::gcroot<EventHandler<MBBS::AI::OpenAI::ChatEventArgs^>^> chatHandler;
        msclr::gcroot<EventHandler<MBBS::AI::OpenAI::ImageEventArgs^>^> imageHandler;
        ChatCallback chatcb;
		ImageCallback imgcb;
    };

    /*----------- ctor -----------*/
    AIWrapper::AIWrapper(const std::string& chatModel, const std::string& imgModel, const std::string& chatPrompt)
    {
        _impl = new Impl;

        _impl->ai = gcnew AIService(gcnew String(chatModel.c_str()), gcnew String(imgModel.c_str()), gcnew String(chatPrompt.c_str()));
        _impl->relay = gcnew Relay(&_impl->chatcb);

        auto ch = gcnew EventHandler<MBBS::AI::OpenAI::ChatEventArgs^>(
            _impl->relay, &Relay::OnManagedMessage);

		auto ih = gcnew EventHandler<MBBS::AI::OpenAI::ImageEventArgs^>(
			_impl->relay, &Relay::OnImageMessage);

        _impl->chatHandler = ch;            
        _impl->ai->OnMessageReceived += ch; 
		_impl->imageHandler = ih;
		_impl->ai->OnImageReceived += ih;
    }

    /*----------- dtor -----------*/
    AIWrapper::~AIWrapper()
    {
        AIService^ ai = static_cast<AIService^>(_impl->ai);
        EventHandler<MBBS::AI::OpenAI::ChatEventArgs^>^ ch =
            static_cast<EventHandler<MBBS::AI::OpenAI::ChatEventArgs^>^>(_impl->chatHandler);
		EventHandler<MBBS::AI::OpenAI::ImageEventArgs^>^ ih =
			static_cast<EventHandler<MBBS::AI::OpenAI::ImageEventArgs^>^>(_impl->imageHandler);

        if (ai != nullptr && ch != nullptr && ih != nullptr)
        {
            ai->OnMessageReceived -= ch;
			ai->OnImageReceived -= ih;
        }
        
        delete _impl;
        _impl = nullptr;
    }

    /*----------- public API -----------*/
    void AIWrapper::StartSession(int channel,
        std::string& userid)
    {
        _impl->ai->StartSession(channel, gcnew String(userid.c_str()));
    }

    void AIWrapper::EndSession(int channel)
    {
		_impl->ai->EndSession(channel);
    }

    void AIWrapper::SetChatCallback(ChatCallback cb)
    {
        _impl->chatcb = std::move(cb);
    }

    void AIWrapper::ClearConversationHistory(int channel)
    { 
        _impl->ai->ClearConversationHistory(channel);
    }

    void AIWrapper::ChatAsync(int channel, std::string const& msg)
    {
        _impl->ai->ChatAsync(channel, gcnew String(msg.c_str()));
    }

	void AIWrapper::SetImageCallback(ImageCallback cb)
	{
		_impl->imgcb = std::move(cb);
	}

	void AIWrapper::GetImageAsync(int channel, std::string const& prompt)
	{
		_impl->ai->GetImageAsync(channel, gcnew String(prompt.c_str()));
	}
}
