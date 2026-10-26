"""Append OBJ geometry, remapping its vertex indices into the destination."""

import sys


def append_obj(destination, source, material=None):
    try:
        with open(destination, "rt") as stream:
            offset = sum(line.startswith("v ") for line in stream)
    except FileNotFoundError:
        offset = 0

    # Prepare the entire addition before opening the destination for writing.
    output = []
    vertices = 0
    with open(source, "rt") as stream:
        for line in stream:
            if line.startswith("v "):
                vertices += 1
                output.append(line)
            elif line.startswith(("o ", "g ")):
                output.append(line)
            elif line.startswith("usemtl "):
                output.append("usemtl %s\n" % material if material is not None else line)
            elif line.startswith("f "):
                tokens = line.partition("#")[0].split()[1:]
                indices = [int(token.split("/")[0]) for token in tokens]
                if len(indices) < 3 or any(index == 0 for index in indices):
                    raise ValueError("invalid OBJ face: %s" % line.rstrip())
                indices = [index if index > 0 else vertices + index + 1 for index in indices]
                output.append("f %s\n" % " ".join(str(offset + index) for index in indices))
    with open(destination, "at") as stream:
        stream.writelines(output)


def main():
    if len(sys.argv) not in (3, 4):
        print("Usage: addOBJ.py result.obj to_add.obj [material_id]")
        return 1
    append_obj(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) == 4 else None)
    return 0


if __name__ == "__main__":
    sys.exit(main())
