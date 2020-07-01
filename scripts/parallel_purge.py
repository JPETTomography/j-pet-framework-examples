#!/usr/bin/env python3
from fnmatch import filter
from os import path, listdir, popen
import argparse
from subprocess import Popen, PIPE
from multiprocessing.dummy import Pool as PoolThread


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

    try:
        import tqdm
    except ImportError:
        print(
            "\033[93m" + "Module tqdm for Python3 not found. Running without progress bar" + "\033[0m")
        progress_bar = False

    if threads > 20:
        print("\033[31m" + "Try not to use more than 20 threads, let others also run analysis." +
              "\033[0m", attrs=["underline"])
        exit()

    if not path.isdir(input_directory):
        print(
            "\033[31m" + "Specified input drectory des not exist. Please check spelling." + "\033[0m")
        exit()

    if input_directory[-1] != "/":
        input_directory += "/"

    list_of_files = filter(listdir(input_directory), "*.root")

    print("\033[32m" + "All checks passed, purging now." + "\033[0m")

    pool = PoolThread(threads)

    def run_macro_parallel(file):
        Popen("root -l -b -q \"purge.C(\\\"{}{}\\\")\"".format(input_directory,
                                                               file), shell=True, stdout=PIPE).wait()

    if progress_bar:
        for _ in tqdm.tqdm(pool.imap(run_macro_parallel, list_of_files), total=len(list_of_files)):
            pass
    else:
        result = pool.map(run_macro_parallel, list_of_files)

    pool.join()
    pool.close()


if __name__ == "__main__":
    main()
