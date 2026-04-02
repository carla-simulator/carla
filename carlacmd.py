import subprocess

def run_command(command : list):
    subprocess.run(command).check_returncode()

def main():
    from argparse import ArgumentParser
    from pathlib import Path
    import glob, os
    from concurrent.futures import ProcessPoolExecutor, as_completed

    CARLA_ROOT = Path(__file__).parent
    BUILD_ROOT_SELECTOR = CARLA_ROOT / 'targets.txt'

    main_parser = ArgumentParser()
    subparser = main_parser.add_subparsers(
        dest='primary_command',
        required=True,
        help='Primary command')
    configure_parser = subparser.add_parser(
        'configure',
        help='Configure CARLA\'s build.')
    
    target_map = {}
    if BUILD_ROOT_SELECTOR.exists():
        with open(BUILD_ROOT_SELECTOR, 'r') as file:
            for line in file.readlines():
                key, command = line.split('::')
                target_map[key] = command.strip().split(' ')
                _ = subparser.add_parser(
                    key,
                    help=f'Build CMake target "{key}"'
                )
    
    configure_parser.add_argument(
        'build_config',
        choices=['debug','development','release']
    )
    main_parser.add_argument('-j', '--jobs', type=int, default=0, help='Max job count.')
    
    ARGV = main_parser.parse_args()
    
    match ARGV.primary_command:
        case 'configure':
            config = str(ARGV.build_config).capitalize()
            run_command(f'cmake --preset {config} -DCMAKE_POLICY_VERSION_MINIMUM=3.5'.split(' '))
            pass
        case _:
            jobs = None if ARGV.jobs == 0 else ARGV.jobs
            if jobs != 0:
                with ProcessPoolExecutor(max_workers=jobs) as pool:
                    futures = []
                    futures.append(
                        pool.submit(
                            run_command,
                            target_map[ARGV.primary_command]))
                    for f in futures:
                        f.result()
            else:
                if ARGV.primary_command in target_map:
                    run_command(target_map[ARGV.primary_command])

if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(e)
    finally:
        pass
