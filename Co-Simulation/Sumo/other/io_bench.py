from subprocess import Popen
import time

text = "a" * 100000
filepath = "./tmp.txt"

start = time.time()
# ----- python -----
# with open(filepath, mode="w") as f:
#     f.write(text)

# ----- shell -----
Popen(f"echo \"{text}\" > {filepath}", shell=True)

# for i in range(0, 1000):
    # Popen(f"echo -e \"{text}\" > {filepath}", shell=True)

    # with open(filepath, mode="w") as f:
    #     f.write(text)
print(f"bench: {time.time() - start}")
