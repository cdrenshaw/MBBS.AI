using OpenAI.Chat;
using System.ClientModel;

namespace MBBS.AI.Wrapper
{
    public class Chat
    {
        public EventHandler<string>? OnMessageReceived;

        ChatClient? _chatClient;
        string _systemMessage = "You are a helpful chat bot.";
        List<ChatMessage> _messages = new();

        public Chat()
        {
            InitializeChat("gpt-4o", _systemMessage);
        }

        public Chat(string model)
        {
            InitializeChat(model, _systemMessage);
        }

        public Chat(string model, string systemMessage)
        {
            InitializeChat(model, systemMessage);
        }

        public void InitializeChat(string model, string systemMessage)
        {
            _chatClient = new(model: model, Environment.GetEnvironmentVariable("OPENAI_API_KEY"));
            _systemMessage = systemMessage;
            _messages.Clear();
            _messages.Add(new SystemChatMessage(systemMessage));
        }

        public async Task ChatAsync(string userMessage)
        {
            string fullResponse = string.Empty;

            _messages.Add(new UserChatMessage(userMessage));
            AsyncCollectionResult<StreamingChatCompletionUpdate> completionUpdates = _chatClient.CompleteChatStreamingAsync(_messages.ToArray());

            try
            {
                await foreach (StreamingChatCompletionUpdate completionUpdate in completionUpdates)
                {
                    if (completionUpdate.ContentUpdate.Count > 0)
                    {
                        string partialResponse = completionUpdate.ContentUpdate[0].Text;
                        OnMessageReceived?.Invoke(this, partialResponse);
                        fullResponse += partialResponse;
                    }
                }
                _messages.Add(new AssistantChatMessage(fullResponse));
            }
            catch (Exception ex)
            {
                OnMessageReceived?.Invoke(this, $"Error: {ex.Message}"); 
            }
        }
    }
}
