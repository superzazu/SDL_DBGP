import argparse


def find_glyph(file, codepoint):
    for line in file:
        code, glyph = line.split(":")
        c = int(code, 16)

        if c == codepoint:
            return glyph
    return None


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Converts UNSCII .hex files into C header file with raw bytes"
    )
    parser.add_argument(
        "-B", "--bytes", help="number of bytes per character (8 for UNSCII-8, 16 for UNSCII-16)", default=8, type=int)
    parser.add_argument("input_file", type=argparse.FileType("r"))
    parser.add_argument(
        "output_file", type=argparse.FileType("w", encoding="UTF-8"))
    args = parser.parse_args()

    output_data = []

    with args.input_file as f:
        for i in range(256):
            c = i.to_bytes(1, "little").decode("latin-1")
            codepoint = ord(c)

            glyph = find_glyph(f, codepoint)
            if glyph is None:
                raise Exception("Glyph '{}' not found".format(c))
            f.seek(0)

            for i in range(0, args.bytes * 2, 2):
                byte = int(glyph[i:i + 2], 16)
                output_data.append(byte)

    with args.output_file as f:
        f.write(
            f"static const unsigned int DBGP_UNSCII{args.bytes}_WIDTH = 8;\n")
        f.write(
            f"static const unsigned int DBGP_UNSCII{args.bytes}_HEIGHT = {args.bytes};\n")
        f.write(
            f"static const unsigned int DBGP_UNSCII{args.bytes}_NB_GLYPHS = 256;\n")
        f.write(
            f"static const char DBGP_UNSCII{args.bytes}[256 * {args.bytes}] = {{")
        f.write(", ".join(map(hex, output_data)))
        f.write("};\n")
