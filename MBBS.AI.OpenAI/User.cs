using OpenAI.Chat;

namespace MBBS.AI.OpenAI
{
    public class User
    {
        internal User(string userId)
        {
            UserId = userId;
        }

        public string UserId { get; set; } = string.Empty;
        public List<ChatMessage> ConversationHistory { get; set; } = new();
    }
}
