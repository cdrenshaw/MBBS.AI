namespace MBBS.AI.OpenAI
{
    public class ImageEventArgs : AIEventArgs
    {
        public ImageEventArgs(int channelId, string userId)
            : base(channelId, userId) { }

        public ImageEventArgs(
            int channelId,
            string userId,
            string message,
            ulong sendTokens = 0,
            ulong recvTokens = 0,
            bool isError = false)
            : base(channelId, userId, message, sendTokens, recvTokens, isError) { }
    }
}
