namespace MBBS.AI.OpenAI
{
    public interface IAIService
    {
        event EventHandler<ChatEventArgs>? OnMessageReceived;
        event EventHandler<ImageEventArgs>? OnImageReceived;
        void StartSession(int channelId, string userId);
        void EndSession(int channelId);
        Task ChatAsync(int channelId, string userMessage);
        void ClearConversationHistory(int channelId);
        Task GetImageAsync(int channelId, string imgPrompt);
    }
}
