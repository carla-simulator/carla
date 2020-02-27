import sys
import re

def main():
    # check parameters
    if len(sys.argv) < 3:
        print("Usage: result.obj to_add.obj [material_id]")
        sys.exit(1)

    #----------------------------------
    # read total of vertex already used
    #----------------------------------
    total_vertex = 0
    try:
        f = open(sys.argv[1], "rt")
        lines = f.readlines()
        for line in lines:
            if line[:2] == "v ":
                total_vertex += 1
    except IOError:
        f = open(sys.argv[1], "wt")
    f.close()

    #--------------------------------------
    # read the min face index of the source
    #--------------------------------------
    min_index = 10000000
    try:
        f = open(sys.argv[2], "rt")
        lines = f.readlines()
        for line in lines:
            if line[:2] == "f ":
                # get the indexes (index[/uv/normal] index[/uv/normal] index[/uv/normal])
                res = re.match(r"f (\d+).*? (\d+).*? (\d+)", line)
                if not res:
                    print("Problem parsing face indexes '%s'" % line)
                    sys.exit(1)
                else:
                    # get the min index
                    if min_index > int(res.groups(0)[0]):
                        min_index = int(res.groups(0)[0])
                    if min_index > int(res.groups(0)[1]):
                        min_index = int(res.groups(0)[1])
                    if min_index > int(res.groups(0)[2]):
                        min_index = int(res.groups(0)[2])
    except IOError:
        min_index = 0
    f.close()

    #----------------
    # read obj to add
    #----------------
    f = open(sys.argv[2], "rt")
    if not f:
        print("Could not open %s file" % sys.argv[2])
        sys.exit(1)
    lines = f.readlines()
    f.close()
    f = open(sys.argv[1], "at")
    for line in lines:
        # object
        if line[:2] == "o ":
            f.write(line)
        # group
        elif line[:2] == "g ":
            f.write(line)
        # vertex
        elif line[:2] == "v ":
            f.write(line)
        # material
        elif line[:2] == "us":
            if len(sys.argv) > 3:
                f.write("usemtl %s\n" % sys.argv[3])
            else:
                f.write(line)
        # face
        elif line[:2] == "f ":
            # get the indexes (index[/uv/normal] index[/uv/normal] index[/uv/normal])
            res = re.match(r"f (\d+).*? (\d+).*? (\d+)", line)
            if not res:
                print("Problem parsing face indexes '%s'" % line)
                sys.exit(1)
            f.write("f %d %d %d\n" % (total_vertex + int(res.groups(0)[0]) - min_index + 1,
                                      total_vertex + int(res.groups(0)[1]) - min_index + 1,
                                      total_vertex + int(res.groups(0)[2]) - min_index + 1))
    f.close()

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('Done.\n')
