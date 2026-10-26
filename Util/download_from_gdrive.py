#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Download big files from Google Drive."""

import argparse
import shutil
import sys
import os
import tempfile

import requests


def sizeof_fmt(num, suffix='B'):
    # https://stackoverflow.com/a/1094933/5308925
    for unit in ['', 'K', 'M', 'G', 'T', 'P', 'E', 'Z']:
        if abs(num) < 1000.0:
            return "%3.2f%s%s" % (num, unit, suffix)
        num /= 1000.0
    return "%.2f%s%s" % (num, 'Yi', suffix)


def print_status(destination, progress):
    message = "Downloading %s...    %s" % (destination, sizeof_fmt(progress))
    empty_space = shutil.get_terminal_size((80, 20)).columns - len(message)
    sys.stdout.write('\r' + message + empty_space * ' ')
    sys.stdout.flush()


def download_file_from_google_drive(id, destination):
    # https://stackoverflow.com/a/39225039/5308925

    def save_response_content(response, destination):
        chunk_size = 32768
        written_size = 0

        destination = os.fspath(destination)
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(mode="wb", delete=False,
                                             dir=os.path.dirname(os.path.abspath(destination))) as f:
                temporary = f.name
                for chunk in response.iter_content(chunk_size):
                    if chunk:  # filter out keep-alive new chunks
                        f.write(chunk)
                        written_size += len(chunk)
                        print_status(destination, written_size)
            os.replace(temporary, destination)
        finally:
            if temporary and os.path.exists(temporary):
                os.remove(temporary)
        print('Done.')

    def get_confirm_token(response):
        for key, value in response.cookies.items():
            if key.startswith('download_warning'):
                return value

        return None

    url = "https://docs.google.com/uc?export=download"

    with requests.Session() as session:
        response = session.get(url, params={'id': id}, stream=True)
        try:
            response.raise_for_status()
            token = get_confirm_token(response)
            if token:
                response.close()
                params = {'id': id, 'confirm': token}
                response = session.get(url, params=params, stream=True)
                response.raise_for_status()
            save_response_content(response, destination)
        finally:
            response.close()


if __name__ == "__main__":

    try:

        argparser = argparse.ArgumentParser(description=__doc__)
        argparser.add_argument(
            'id',
            help='Google Drive\'s file id')
        argparser.add_argument(
            'destination',
            help='destination file path')
        args = argparser.parse_args()

        download_file_from_google_drive(args.id, args.destination)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
