using MBBS.AI.OpenAI;
using OpenAI.Chat;
using System.ClientModel;

namespace MBBS.AI.Wrapper
{
    public class Chat
    {
        public EventHandler<ChatEventArgs>? OnMessageReceived;

        ChatClient? _chatClient;
        Dictionary<int, User> _users = new();
        string _systemPrompt;
        string _model;
        
        // ctor
        public Chat(string model, string prompt) 
        {
            _model = model;
            _systemPrompt = prompt;
            _chatClient = new(model: _model, Environment.GetEnvironmentVariable("OPENAI_API_KEY"));
        }

        public void StartChatSession(int channelId, string userId)
        {
            _users.Add(channelId, new User(userId));
        }

        public void EndChatSession(int channelId)
        {
            if (_users.ContainsKey(channelId))
                _users.Remove(channelId);
        }

        public async Task ChatAsync(int channelId, string userMessage)
        {
            if (!_users.ContainsKey(channelId))
            { 
                OnMessageReceived?.Invoke(this, new ChatEventArgs(
                    channelId,
                    userId: string.Empty,
                    message: "Chat session not started.",
                    isError: true,
                    isFinal: true));
                return;
            }

            User user = _users[channelId];

            if (_chatClient == null)
            {
                OnMessageReceived?.Invoke(this, new ChatEventArgs(
                    channelId,
                    user.UserId,
                    message: "Chat client not initialized.", 
                    isError: true, 
                    isFinal: true));
                return;
            }

            // Add the user's tag in the request
            user.ConversationHistory.Add(new UserChatMessage(userMessage) { ParticipantName = user.UserId });
            
            // TODO: summarize the first few messages if over a certain number

            AsyncCollectionResult<StreamingChatCompletionUpdate> completionUpdates =
                _chatClient.CompleteChatStreamingAsync(user.ConversationHistory.ToArray());

            string fullResponse = string.Empty;
            try
            {
                await foreach (StreamingChatCompletionUpdate completionUpdate in completionUpdates)
                {
                    // Stop if user input is refused
                    if (!string.IsNullOrEmpty(completionUpdate.RefusalUpdate))
                    {
                        OnMessageReceived?.Invoke(this, new ChatEventArgs(
                            channelId,
                            user.UserId,
                            message: completionUpdate.RefusalUpdate,
                            isError: true));
                        break;
                    }

                    // Send the message to the UI
                    if (completionUpdate.ContentUpdate.Count > 0)
                    {
                        string partialResponse = completionUpdate.ContentUpdate[0].Text;
                        OnMessageReceived?.Invoke(this, new ChatEventArgs(
                            channelId,
                            user.UserId,
                            message: partialResponse));
                        fullResponse += partialResponse;
                    }

                    // Stop if user token limit is reached
                    if (completionUpdate.FinishReason.HasValue && 
                        completionUpdate.FinishReason == ChatFinishReason.Length)
                    {
                        OnMessageReceived?.Invoke(this, new ChatEventArgs(
                            channelId,
                            user.UserId,
                            message: "Token limit reached.",
                            isError: true));
                        break;
                    }
                }

                OnMessageReceived?.Invoke(this, new ChatEventArgs(
                    channelId,
                    user.UserId,
                    message: string.Empty,
                    isFinal: true));

                user.ConversationHistory.Add(new AssistantChatMessage(fullResponse));
            }
            catch (Exception ex)
            {
                OnMessageReceived?.Invoke(this, new ChatEventArgs(
                    channelId,
                    user.UserId,
                    message: ex.Message,
                    isError: true,
                    isFinal: true));
            }
        }

        public void ClearConversationHistory(int channelId)
        {
            if (!_users.ContainsKey(channelId))
                return;
            _users[channelId].ConversationHistory.Clear();
            _users[channelId].ConversationHistory.Add(new SystemChatMessage(_systemPrompt));
        }
    }
}
