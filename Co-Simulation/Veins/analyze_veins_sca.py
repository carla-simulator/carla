import argparse
import matplotlib
import os
import shutil


from fpdf import FPDF
from pylab import title, figure, xlabel, ylabel, xticks, bar, legend, axis, savefig


from util.func import (
    data_from_json,
)

def readlines(file_path):
    with open(file_path) as f:
        return f.readlines()


class ScaFileHandler:
    def __init__(self, sca_file_path, analyze_results_dir, config_file_name):
        self.simulation_identifier, self.config, self.node2results, self.all_modules, self.all_metrics = self.__parse(sca_file_path)
        self.analyze_results_dir = analyze_results_dir + self.simulation_identifier
        self.config_file_name = config_file_name

        if os.path.exists(self.analyze_results_dir):
            shutil.rmtree(self.analyze_results_dir)
        os.mkdir(self.analyze_results_dir)

        self.__save_config()

    def print(self):
        for node_id, results in self.node2results.items():
            ReceivedBroadcasts = 0
            TotalLostPackets = 0

            for result in results:
                if result["metric"] == "ReceivedBroadcasts":
                    ReceivedBroadcasts = float(result["value"])
                elif result["metric"] == "TotalLostPackets":
                    TotalLostPackets = float(result["value"])
                else:
                    continue

            PDR = ReceivedBroadcasts / (ReceivedBroadcasts + TotalLostPackets) if (ReceivedBroadcasts + TotalLostPackets) != 0 else None
            print(f"node_id: {node_id}, PDR: {PDR}")

    def pdr(self):

        pass


    def __parse(self, sca_file_path):
        lines = readlines(sca_file_path)
        simulation_identifier = "non-identifier"
        config = ""
        all_metrics = []
        all_modules = []
        node2results = {}

        for l in lines:
            if l.split(" ")[0] == "run":
                """
                ----- config and simulation_identifier-----
                The line is formmated as follows:
                run {simulation_identifier}
                """
                simulation_identifier = l.split(" ")[1]
                config = config + l

            elif l.split(" ")[0] == "scalar":
                """
                ----- scalar -----
                Scalar is formatted as follows:
                scalar {scnario name}.node[{node_id}].module1.module2.*** {metric} {velue}
                """
                node_id, modules, metric, value = self.__scalar_parser(l)

                if node_id not in node2results.keys():
                    node2results[node_id] = []
                node2results[node_id].append({"metric": metric, "modules": modules, "value": value})

                all_modules = list(set(all_modules) | set(modules))
                all_metrics = list(set(all_metrics) | set([metric]))

            else:
                """
                ----- config -----
                """
                config = config + l

        return simulation_identifier, config, node2results, all_modules, all_metrics


    def __save_config(self):
        with open(self.analyze_results_dir + "/" + self.config_file_name, mode="w") as f:
            f.write(self.config)


    def __scalar_parser(self, scalar_line):
        node_id = scalar_line.split(" ")[1].split(".")[1]
        modules = [scalar_line.split(" ")[1].split(".")[index] for index in range(2, len(scalar_line.split(" ")[1].split(".")))]
        metric = scalar_line.split(" ")[2]
        value = scalar_line.split(" ")[3]

        return node_id, modules, metric, value


class Main:
    def __init__(self, args, env, sca_file_path, analyze_results_dir, config_file_name):
        self.sca_handler = ScaFileHandler(sca_file_path, analyze_results_dir, config_file_name)

    def run(self):
        self.sca_handler.print()


if __name__ == "__main__":
    env    = data_from_json("./env/analyze_veins_sca.json")
    parser = argparse.ArgumentParser(description='This script is sca file analyzer.')

    parser.add_argument('--analyze_results_dir', default=env["analyze_results_dir"])
    parser.add_argument('--config_file_name', default=env["config_file_name"])
    parser.add_argument('--sca_file_path', default=env["sca_file_path"])

    args = parser.parse_args()
    Main(args, env, args.sca_file_path, args.analyze_results_dir, args.config_file_name).run()
