using MBBS.AI.OpenAI;
using OpenAI.Chat;
using OpenAI.Images;
using System.ClientModel;
using System.Collections.Concurrent;

namespace MBBS.AI.Wrapper
{
    public class AIService : IAIService
    {
        public event EventHandler<ChatEventArgs>? OnMessageReceived;
        public event EventHandler<ImageEventArgs>? OnImageReceived;

        readonly ChatClient _chatClient;
        readonly ImageClient _imageClient;

        ConcurrentDictionary<int, User> _users = new();
        string _chatPrompt;
        string _chatModel;
        string _imageModel;

        // ctor
        public AIService(string chatModel, string imageModel, string chatPrompt) 
        {
            _chatModel = chatModel;
            _imageModel = imageModel;
            _chatPrompt = chatPrompt;
            _chatClient = new(model: _chatModel, Environment.GetEnvironmentVariable("OPENAI_API_KEY"));
            _imageClient = new(model: _imageModel, Environment.GetEnvironmentVariable("OPENAI_API_KEY"));
        }

        public void StartSession(int channelId, string userId)
        {
            if (_users.ContainsKey(channelId))
                return;

            User user = new(userId);
            user.ConversationHistory.Add(new SystemChatMessage(_chatPrompt));
            _users.TryAdd(channelId, user);
        }

        public void EndSession(int channelId)
        {
            if (!_users.ContainsKey(channelId))
                return;

            _users.TryRemove(channelId, out User? user);
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
            _users[channelId].ConversationHistory.Add(new SystemChatMessage(_chatPrompt));
        }

        public async Task GetImageAsync(int channelId, string imgPrompt)
        {
            if (!_users.ContainsKey(channelId))
            {
                OnImageReceived?.Invoke(this, new ImageEventArgs(
                    channelId,
                    userId: string.Empty,
                    message: "Image session not started.",
                    isError: true));
                return;
            }

            ImageGenerationOptions options = new()
            {
                Quality = GeneratedImageQuality.High,
                Size = GeneratedImageSize.W1024xH1024,
                Style = GeneratedImageStyle.Vivid,
                ResponseFormat = GeneratedImageFormat.Bytes
            };

            try
            {
                ClientResult<GeneratedImage> result = await _imageClient.GenerateImageAsync(imgPrompt, options);
                string ansi = ImageProcessor.ToAnsi(result.Value.ImageBytes.ToArray());

                foreach (string part in ImageProcessor.ChunkAnsi(ansi, 240)) 
                {
                    OnImageReceived?.Invoke(this, new ImageEventArgs(
                    channelId,
                    _users[channelId].UserId,
                    message: part));
                }
            }
            catch (Exception ex)
            {
                OnImageReceived?.Invoke(this, new ImageEventArgs(
                    channelId,
                    _users[channelId].UserId,
                    message: ex.Message,
                    isError: true));
            }

            return;
        }
    }
}
