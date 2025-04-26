using Microsoft.ClearScript.V8;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;
using SixLabors.ImageSharp.Processing;
using System.Reflection;

namespace MBBS.AI.OpenAI
{
    internal class ImageProcessor
    {
        static readonly V8ScriptEngine _js = LoadEngine();

        internal static string ToAnsi(byte[] png, int maxW = 160, int maxH = 90, bool unicode = true)
        {
            // 1. decode & resize in .NET
            using Image<Rgba32> img = Image.Load<Rgba32>(png);
            img.Mutate(c => c.Resize(new ResizeOptions
            {
                Mode = ResizeMode.Max,
                Size = new Size(maxW, maxH)
            }));

            int w = img.Width, h = img.Height;
            var pixelBytes = new byte[w * h * 4];
            img.CopyPixelDataTo(pixelBytes); 

            // 2. expose host data to JS
            _js.AddHostObject("rgba", pixelBytes);
            _js.AddHostObject("opts", new { colours = "false", unicode, width = w, height = h });

            // 3. run the pure JS and get the ANSI string
            return _js.Evaluate("pixelsToAnsi(rgba, opts);").ToString() ?? string.Empty;
        }

        static V8ScriptEngine LoadEngine()
        {
            var eng = new V8ScriptEngine();               
            eng.Execute(GetScript("RgbToAnsi.js"));
            eng.Execute(GetScript("ImageToAnsiProcessor.js"));
            return eng;
        }

        static string GetScript(string name)
        {
            var asm = Assembly.GetExecutingAssembly();
            var path = $"MBBS.AI.OpenAI.Scripts.{name}";
            using var s = asm.GetManifestResourceStream(path)!;
            using var r = new StreamReader(s);
            return r.ReadToEnd();
        }
    }
}
