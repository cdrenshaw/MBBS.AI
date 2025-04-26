namespace MBBS.AI.OpenAI
{
    public class ChatEventArgs : AIEventArgs
    {
        public ChatEventArgs(int channelId, string userId)
            : base(channelId, userId) { }

        public ChatEventArgs(
            int channelId, 
            string userId, 
            string message,
            ulong sendTokens = 0, 
            ulong recvTokens = 0, 
            bool isFinal = false, 
            bool isError = false)
            : base(channelId, userId, message, sendTokens, recvTokens, isFinal, isError)
        {
        }
    }
}
