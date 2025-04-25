# MBBS AI
A Chat GPT module for Major BBS v10

## Installation
1. If you don't already have an OpenAI API key, you can sign up and get one at [platform.openai.com](https://platform.openai.com).  Keep in mind that API usage costs money and you'll need to fund your account with a credit card before you can use your API key.
2. On your MBBS server, install the latest 32-bit x86 versions of the [.NET 9 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-9.0.4-windows-x86-installer) and the [Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x86.exe).
3. Create a User Environment Variable for the user that runs MBBS.  With PowerShell, for example: `[Environment]::SetEnvironmentVariable('OPENAI_API_KEY', 'your api key')`
4. Compile the solution using Visual Studio in Release mode for x86/win32 so it includes vcruntime and ucrtbase.
5. From the solution folder, run: `dotnet publish .\MBBS.AI.OpenAI\MBBS.AI.OpenAI.csproj -c Release -f net9.0 -r win-x86 --no-self-contained -o .\Release`
6. Copy the following files from the \MBBS.AI\Release directory to your MBBS directory:
    - ijwhost.dll
    - MBBS.AI.OpenAI.deps.json
    - MBBS.AI.OpenAI.dll
    - MBBS.AI.Wrapper.deps.json
    - MBBS.AI.Wrapper.dll
    - MBBS.AI.Wrapper.runtimeconfig.json
    - MBBS.AI.DAT
    - MBBSAI.dll
    - MBBSAI.MDF
    - MBBSAI.MSG
    - MBBSAI.VIR
    - OpenAI.dll
    - System.ClientModel.dll
    - System.Memory.Data.dll
7. Restart MBBS.
