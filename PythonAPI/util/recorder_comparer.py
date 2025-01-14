#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Compare two recorder files (gotten from 'show_recorder_file_info') line by line"""

import argparse

def main():

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-f1', '--file1',
        help='recorder filename (test1.txt)')
    argparser.add_argument(
        '-f2', '--file2',
        help='recorder filename (test2.txt)')

    args = argparser.parse_args()

    identical = True
    FRAME_SPACES = 5
    TIME_SPACES = 7

    with open(args.file1, 'r') as fd:
        recorder_1_str = fd.read()
    with open(args.file2, 'r') as fd:
        recorder_2_str = fd.read()

    list_of_rows_1 = recorder_1_str.split("\n")[3:]
    list_of_rows_2 = recorder_2_str.split("\n")[3:]
    len_1 = len(list_of_rows_1)
    len_2 = len(list_of_rows_2)

    if len_1 < 10 or len_2 < 10:
        raise ValueError('Recorder have less than 10 lines. This might be due to an error')

    ignored_ids_1 = []
    ignored_ids_2 = []

    frame_1 = 0
    frame_2 = 0
    time_1 = 0
    time_2 = 0

    print("")
    try:
        for i in range(len_1):
            row1 = list_of_rows_1[i].strip()
            row2 = list_of_rows_2[i].strip()

            row1_id = None
            row2_id = None

            # Get the recorder's frame info. Part of the output when a difference is detected
            if row1.startswith('Frame '):
                frame_1 = row1.split(' ')[1]
                time_1 = row1.split(' ')[3]
            if row2.startswith('Frame '):
                frame_2 = row2.split(' ')[1]
                time_2 = row2.split(' ')[3]

            # Ignore some lines
            if row1.startswith("Parenting") and row2.startswith("Parenting"):  # Based on ids, which vary
                continue
            elif row1.startswith("Collision") and row2.startswith("Collision"):  # Based on ids, which vary
                continue
            elif row1.startswith("Destroy"):  # No more info apart from the id, which vary
                continue
            elif row1.startswith("Current platform time:"):  # Real time, so it can vary without affecting determinism
                continue

            # Ignoring the Id part, as it can differ between simulations
            elif row1.startswith("Id:"):
                _, row1_id, *row1_info = row1.split(" ")
                _, row2_id, *row2_info = row2.split(" ")

            # Same with Create and Destroy, but 
            elif row1.startswith("Create"):
                _, row1_id, *row1_info = row1.split(" ")
                _, row2_id, *row2_info = row2.split(" ")

                row1_id = row1_id[:-1]
                row2_id = row2_id[:-1]

                # Ignore the spectator, as it can vary throughout simulations
                if 'spectator' in row1:
                    ignored_ids_1.append(row1_id)
                if 'spectator' in row2:
                    ignored_ids_2.append(row2_id)

            else:
                row1_info = row1.split(" ")
                row2_info = row2.split(" ")

            # Check if the ID has to be ignored
            if row1_id and row2_id and row1_id in ignored_ids_1 and row2_id in ignored_ids_2:
                continue

            # Compare the data
            if row1_info != row2_info:
                identical = False
                print(f" --------------------------------------------------------------------------------- ")
                print(f" Line being compared:  \033[1m{i+4}\033[0m")
                print(f" [Frame \033[1m{frame_1}{' ' * max(0, FRAME_SPACES-len(frame_1))}\033[0m - Time \033[1m{time_1}{' ' * max(0, TIME_SPACES-len(time_1))}\033[0m] -> (\033[1m" + row1 + "\033[0m)")
                print(f" [Frame \033[1m{frame_2}{' ' * max(0, FRAME_SPACES-len(frame_2))}\033[0m - Time \033[1m{time_2}{' ' * max(0, TIME_SPACES-len(time_2))}\033[0m] -> (\033[1m" + row2 + "\033[0m)")
                input()

        if identical:
            print("Both files are identical")
        else:
            print("No more lines to compare")

    except KeyboardInterrupt:
        pass

    except Exception as e:
        print(f"Detected an exception at line {i+4}")
        print(e)

if __name__ == '__main__':
    main()
