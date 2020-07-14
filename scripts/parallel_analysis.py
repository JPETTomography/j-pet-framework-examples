#!/usr/bin/env python3

from multiprocessing.dummy import Pool as PoolThread
from os import listdir, system, path
from sys import exit
from fnmatch import filter
import argparse
from sys import exit

ALLOWED_TYPES = ["root", "mcGeant", "hld", "zip", "scope"]

def main():
    parser = argparse.ArgumentParser(
        "Python script to analyze files in parallel")
    parser.add_argument("executable", type=str,
                        help="Executable you want to run")
    parser.add_argument("-i", "--input", required=True, type=str, nargs="+",
                        help="Path to the directories you want to analyze")
    parser.add_argument("-r", "--run-id", required=True, type=str,
                        help="Number of run which you are analyzing")

    parser.add_argument("-e", "--extension", required=False, type=str, default="*",
                        help="Extention of files you want to analyze")

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
    input_directories = args["input"]
    output_directory = args["output"]
    progress_bar = args["progress_bar"]
    run_id = args["run_id"]
    file_type = args["type"]
    threads = args["number_of_threads"]
    extension = args["extension"]

    try:
        import tqdm
    except ImportError:
        print(
            "\033[93m" + "Module tqdm for Python3 not found. Running without progress bar" + "\033[0m")
        progress_bar = False


    run_id_setup = get_run_id_setup_mapping(run_id)


    if threads > 20:
        print(
            "\033[31m" + "Try not to use more than 20 threads, let others also run analysis." + "\033[0m")

    for directory in input_directories:
        if not path.isdir(directory):
            print(
                "\033[31m" + "Directory {} input drectory des not exist. Please check spelling.".format(directory) + "\033[0m")
            exit()

    input_directories = [directory + "/" if directory[-1] !=
                         "/" else directory for directory in input_directories]

    if output_directory is not None:
        if not path.isdir(output_directory):
            print(
                "\033[31m" + "Specified output drectory des not exist. Please check spelling or create a directory." + "\033[0m")
            exit()

        if output_directory[-1] != "/":
            output_directory += "/"

    if file_type != "root":
        if file_type not in ALLOWED_TYPES:
            print("\033[31m" + "Specified file type is not valid. Please check if it's one of the following: \n{}".format(
                ", ".join(ALLOWED_TYPES)) + "\033[0m")
            exit()

    files_needed_for_analysis = [
        "userParams.json", "conf_trb3.xml", "detectorSetupRun{}.json".format(run_id_setup)]

    needed_files_present = True

    for file in files_needed_for_analysis:
        if not path.isfile(file):
            print(
                "\033[31m" + "File {} does not exist in current directory.".format(file) + "\033[0m")
            needed_files_present = False

    if not needed_files_present:
        exit()

    supported_extensions = ["*", "hld", "hld.root", "tslot.calib.root", "raw.sig.root",
                            "phys.sig.root", "hits.root", "unk.evt.root", "cat.evt.root"]

    if extension not in supported_extensions:
        print("\033[31m" + "Specified file extension is not valid. Please check if it's one of the following: \n{}".format(
            ", ".join(supported_extensions[1:])) + "\033[0m")
        exit()

    if output_directory is not None:
        def run_analysis_parallel(filename):
            system("./{} -t root -f {} -p conf_trb3.xml -u userParams.json -i {} -l detectorSetupRun{}.json -o {}".format(
                executable, filename, run_id, run_id_setup, output_directory))

    else:
        def run_analysis_parallel(filename):
            system("./{} -t root -f {} -p conf_trb3.xml -u userParams.json -i {} -l detectorSetupRun{}.json".format(
                executable, filename, run_id, run_id_setup))

    list_of_files = []

    for directory in input_directories:
        for file in filter(listdir(directory), "*.{}".format(extension)):
            list_of_files.append(directory + file)

    print("\033[32m" + "All checks passed, running analysis now." + "\033[0m")

    pool = PoolThread(threads)

    if progress_bar:
        for _ in tqdm.tqdm(pool.imap(run_analysis_parallel, list_of_files), total=len(list_of_files)):
            pass
    else:
        results = pool.map(run_analysis_parallel, list_of_files)

    pool.close()
    pool.join()


def get_run_id_setup_mapping(run_id):
    run6_mapping = {"61": "6A",
                    "62": "6B",
                    "63": "6C",
                    "64": "6D"}

    if run_id[0] == "6":
        run_id_setup = run6_mapping[run_id]
    else:
        run_id_setup = run_id


if __name__ == "__main__":
    main()
