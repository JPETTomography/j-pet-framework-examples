#!/usr/bin/env python3

from multiprocessing.dummy import Pool as PoolThread
from os import listdir, system, path
from sys import exit
from fnmatch import filter
import argparse

try:
    from termcolor import colored
except ImportError:
    print("\033[93m" + "Please instal module termcolor for Python3. \n \
run command: sudo apt-get install python3-termcolor \n \
or equivalent on your operating system" + "\033[0m")
    exit()
try:
    import tqdm
except ImportError:
    print("\033[93m" + "Please instal module tqdm for Python3. \n \
run command: sudo apt-get install python3-tqdm \n \
or equivalent on your operating system" + "\033[0m")
    exit()


def main():
    parser = argparse.ArgumentParser(
        "Python script to analyze files in parallel")
    parser.add_argument("executable", type=str,
                        help="Executable you want to run")
    parser.add_argument("-i", "--input", required=True, type=str,
                        help="Path to the directory you want to analyze")
    parser.add_argument("-r", "--run-id", required=True, type=str,
                        help="Number of run which you are analyzing")

    parser.add_argument("-t", "--type", required=False, type=str, default="root",
                        help="Path to the directory you want to analyze")
    parser.add_argument("-o", "--output", required=False,
                        help="Path to the output directory in which you want to save analyzed files")
    parser.add_argument("-p", "--progress-bar", action="store_false",
                        help="Using this option turns progress bar off")
    parser.add_argument("-n", "--number-of-threads", required=False, default=20, type=int,
                        help="Number of threads to run simultaneously")

    args = vars(parser.parse_args())

    executable = args["executable"]
    input_directory = args["input"]
    output_directory = args["output"]
    progress_bar = args["progress_bar"]
    run_id = args["run_id"]
    file_type = args["type"]
    threads = args["number_of_threads"]

    run_id_setup = run_id

    if run_id[0] == "6":
        if run_id[1] == "1":
            run_id_setup = "6A"
        elif run_id[1] == "2":
            run_id_setup = "6B"
        elif run_id[1] == "3":
            run_id_setup = "6C"
        elif run_id[1] == "4":
            run_id_setup = "6D"

    if threads > 20:
        print(colored("Try not to use more than 20 threads, let others also run analysis.",
                      "red", attrs=["underline"]))

    if not path.isdir(input_directory):
        print(colored(
            "Specified input drectory des not exist. Please check spelling.", "red"))
        exit()

    if input_directory[-1] != "/":
        input_directory += "/"

    if output_directory is not None:
        if not path.isdir(output_directory):
            print(colored(
                "Specified output drectory des not exist. Please check spelling or create a directory.", "red"))
            exit()

        if output_directory[-1] is not "/":
            output_directory += "/"

    if file_type != "root":
        allowed_types = ["root", "mcGeant", "hld", "zip", "scope"]
        if file_type not in allowed_types:
            print(colored("Specified file type is not valid. Please check if it's one of the following: {}".format(
                ", ".join(allowed_types)), "red"))
            exit()

    files_needed_for_analysis = [
        "userParams.json", "conf_trb3.xml", "detectorSetupRun{}.json".format(run_id)]
    needed_files_present = True
    for file in files_needed_for_analysis:
        if not path.isfile(file):
            print(
                colored("File {} does not exist in current directory.".format(file), "red"))
            needed_files_present = False

    if not needed_files_present:
        exit()

    if output_directory is not None:
        def run_analysis_parallel(filename):
            print("./{} -t root -f {}{} -p conf_trb3.xml -u userParams.json -i {} -l detectorSetupRun{}.json -o {}".format(
                executable, input_directory, filename, run_id, run_id_setup, output_directory))

    else:
        def run_analysis_parallel(i):
            print("./{} -t root -f {}{} -p conf_trb3.xml -u userParams.json -i {} -l detectorSetupRun{}.json".format(
                executable, input_directory, filename, run_id, run_id_setup))

    list_of_files = listdir(input_directory)

    print(colored("All checks passed, running analysis now.", "green"))

    pool = PoolThread(threads)

    if progress_bar:
        for _ in tqdm.tqdm(pool.imap(run_analysis_parallel, list_of_files), total=len(list_of_files)):
            pass
    else:
        results = pool.map(run_analysis_parallel, list_of_files)

    pool.close()
    pool.join()


if __name__ == "__main__":
    main()
