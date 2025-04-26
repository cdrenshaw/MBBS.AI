// imageToAnsiProcessor.pure.ts  (compile to JS)
import rgbToTerm from './rgbToAnsi';

export interface Options {
    colours: 'true' | 'false';
    unicode: boolean;
    width: number;
    height: number;
}

export function pixelsToAnsi(
    rgba: Uint8Array,            // byte[] from C#
    opts: Options
): string {
    const { width, height, unicode } = opts;
    const stride = width * 4;
    let out = '';

    const isTransparent = (_: number, __: number, ___: number, a: number) => a < 13;
    const rgbaToAnsi = (r: number, g: number, b: number, a: number, fg = false) =>
        a < 13
            ? fg ? '39' : '49'
            : `${fg ? 38 : 48};5;${rgbToTerm(r, g, b)}`;

    for (let y = 0; y < height; y++) {
        for (let x = 0; x < width; x++) {
            const i = y * stride + x * 4;
            const r = rgba[i], g = rgba[i + 1], b = rgba[i + 2], a = rgba[i + 3];

            out += `\x1b[${rgbaToAnsi(r, g, b, a)}m  `;
        }
        out += '\x1b[m\n';
    }
    return out;
}

(globalThis as any).pixelsToAnsi = pixelsToAnsi;  // expose to C#
