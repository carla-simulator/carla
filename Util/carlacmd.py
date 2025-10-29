
def run_command(command : str):
    command.split(' ')

def main():
    from argparse import ArgumentParser
    from pathlib import Path
    import subprocess
    from concurrent.futures import ProcessPoolExecutor, as_completed

    here = Path(__file__).parent
    target_map = {}
    with open(here / 'targets.txt', 'r') as file:
        for line in file.readlines():
            key, command = line.split(':')
            target_map[key] = command.strip()
    
    parser = ArgumentParser()
    for k, v in target_map.items():
        parser.add_argument(k, help=f'Run "{v}"')
    parser.add_argument('-j', '--jobs', type=int, default=0, help='Max job count.')
    parser.add_argument('-p', '--parallel', action='store_true')
    argv = parser.parse_args()
    if argv.parallel:
        jobs = None if parser.jobs == 0 else parser.jobs
        with ProcessPoolExecutor(max_workers=jobs) as pool:
            futures = [pool.submit(run_command, v) for k, v in target_map.items()]
            for f in futures:
                f.result()
    else:
        for k, v in target_map.items():
            if k in argv.__dict__:
                run_command(v)

if __name__ == '__main__':
    main()
