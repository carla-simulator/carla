import argparse


from util.func import (
    data_from_json,
)


class ScaFileHandler:
    def __init__(self):
        pass


    def parse(self):
        pass


class Main:
    def __init__(self, args, env):
        pass


    def run(self):
        pass


if __name__ == "__main__":
    env    = data_from_json("./env/analyze_veins_sca.json")
    parser = argparse.ArgumentParser(description='This script is sca file analyzer.')

    parser.add_argument('--sca_file_path', default=env["sca_file_path"])

    args = parser.parse_args()
    Main(args, env).run()
