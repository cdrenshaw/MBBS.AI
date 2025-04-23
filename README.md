## MBBS.AI
A Chat GPT module for Major BBS v10

On your MBBS server, install the latest dotnet 9 runtime and the latest VC++ redistributable.  Make
sure that they are both the x86 version.  Also create an environment variable called OPENAI_API_KEY
with your OpenAI API key.

Compile the solution in Release mode for x86/win32 to include vcruntime and ucrtbase.

Run `dotnet publish MBBS.AI.OpenAI.csproj -c Release -f net9.0 -r win-x86 --no-self-contained -o ..\deploy`

Copy the following files to your MMBS directory (they will all be in the \Release or \deploy directories):

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
