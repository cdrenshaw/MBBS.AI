namespace MBBS.AI.OpenAI
{
    public class ChatEventArgs : EventArgs
    {
        int _channelId = -1;
        string _userId = string.Empty;

        public ChatEventArgs(int channelId, string userId) { }

        public ChatEventArgs(int channelId, string userId, string message, ulong sendTokens = 0, ulong recvTokens = 0, bool isFinal = false, bool isError = false)
        {
            _channelId = channelId;
            _userId = userId;
            Message = message;
            SendTokens = sendTokens;
            RecvTokens = recvTokens;
            IsError = isError;
            IsFinal = isFinal;
        }

        public int ChannelId => _channelId;
        public string UserId => _userId;
        public string Message { get; set; } = string.Empty;
        public ulong SendTokens { get; set; } = 0;
        public ulong RecvTokens { get; set; } = 0;
        public bool IsError { get; set; } = false;
        public bool IsFinal { get; set; } = false;
    }
}
