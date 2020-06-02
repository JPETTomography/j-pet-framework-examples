#!/usr/bin/env python3
from fnmatch import filter
from os import path, listdir, popen
import argparse

from multiprocessing.dummy import Pool as PoolThread

try:
    from termcolor import colored
except ImportError:
    print("\033[93m" + "Please instal module termcolor for Python3. \n \
run command: sudo apt-get install python3-termcolor \n \
or equivalent on your operating system" + "\033[0m")
try:
    import tqdm
except ImportError:
    print("\033[93m" + "Please instal module tqdm for Python3. \n \
run command: sudo apt-get install python3-tqdm \n \
or equivalent on your operating system" + "\033[0m")


def main():
    parser = argparse.ArgumentParser(
        "Python script to purge root files in parallel")
    parser.add_argument("directory", type=str,
                        help="Directory in which you want to purge files.")
    parser.add_argument("-p", "--progress-bar", action="store_false", required=False,
                        help="Using this option turns progress bar off")
    parser.add_argument("-n", "--number-of-threads", required=False, default=20, type=int,
                        help="Number of threads to run simultaneously")

    args = vars(parser.parse_args())

    input_directory = args["directory"]
    progress_bar = args["progress_bar"]
    threads = args["number_of_threads"]

    if threads > 20:
        print(colored(
            "Try not to use more than 20 threads, let others also run analysis.", "red", attrs=["underline"]))
        exit()

    if not path.isdir(input_directory):
        print(colored(
            "Specified input drectory des not exist. Please check spelling.", "red"))
        exit()

    if input_directory[-1] != "/":
        input_directory += "/"

    list_of_files = filter(listdir(input_directory), "*.root")

    pool = PoolThread(threads)

    def run_macro_parallel(file):
        popen("root -l -b -q \"purge.C(\\\"{}{}\\\")\"".format(input_directory, file))

    if progress_bar:
        for _ in tqdm.tqdm(pool.imap(run_macro_parallel, list_of_files), total=len(list_of_files)):
            pass
    else:
        result = pool.map(run_macro_parallel, list_of_files)

    pool.close()
    pool.join()


if __name__ == "__main__":
    main()
