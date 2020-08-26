#!/usr/bin/env python3

from multiprocessing.dummy import Pool as PoolThread
from os import listdir, system, path
import sys
from fnmatch import filter
import argparse

try:
    import tqdm
except ImportError:
    print(
        "\033[93m" + "Module tqdm for Python3 not found. Running without progress bar" + "\033[0m")

    class tqdm():

        @staticmethod
        def tqdm(args):
            pass

ALLOWED_TYPES = ["root", "mcGeant", "hld", "zip", "scope"]
SUPPORTED_EXTENSIONS = ["root", "hld", "hld.root", "tslot.calib.root", "raw.sig.root",
                        "phys.sig.root", "hits.root", "unk.evt.root", "cat.evt.root"]


def are_valid_args(threads, input_directories, output_directory, file_type, run_id_setup, extension):

    if threads > 20:
        print(
            "\033[31m" + "Try not to use more than 20 threads, let others also run analysis." + "\033[0m")

    for directory in input_directories:
        if not path.isdir(directory):
            print(
                "\033[31m" + "Directory {} input drectory des not exist. Please check spelling.".format(directory) + "\033[0m")
            return False

    if output_directory is not None:
        if not path.isdir(output_directory):
            print(
                "\033[31m" + "Specified output drectory des not exist. Please check spelling or create a directory." + "\033[0m")
            return False

    if file_type != "root":
        if file_type not in ALLOWED_TYPES:
            print("\033[31m" + "Specified file type is not valid. Please check if it's one of the following: \n{}".format(
                ", ".join(ALLOWED_TYPES)) + "\033[0m")
            return False

    files_needed_for_analysis = [
        "userParams.json", "conf_trb3.xml", "detectorSetupRun{}.json".format(run_id_setup)]

    needed_files_present = True

    for fname in files_needed_for_analysis:
        if not path.isfile(fname):
            print(
                "\033[31m" + "File {} does not exist in current directory.".format(fname) + "\033[0m")
            needed_files_present = False

    if not needed_files_present:
        return False

    if extension not in SUPPORTED_EXTENSIONS:
        print("\033[31m" + "Specified file extension is not valid. Please check if it's one of the following: \n{}".format(
            ", ".join(SUPPORTED_EXTENSIONS[1:])) + "\033[0m")
        return False

    return True


def main():
    parser = argparse.ArgumentParser(
        "Python script to analyze files in parallel")
    parser.add_argument("executable", type=str,
                        help="Executable you want to run")
    parser.add_argument("-i", "--input", required=True, type=str, nargs="+",
                        help="Path to the directories you want to analyze")
    parser.add_argument("-r", "--run-id", required=True, type=str,
                        help="Number of run which you are analyzing")

    parser.add_argument("-e", "--extension", required=False, type=str, default="root",
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

    input_directories = [directory + "/" if directory[-1] !=
                         "/" else directory for directory in input_directories]

    if output_directory:
        if output_directory[-1] != "/":
            output_directory += "/"

    run_id_setup = get_run_id_setup_mapping(run_id)

    if not are_valid_args(threads, input_directories, output_directory, file_type, run_id_setup, extension):
        sys.exit()

    list_of_params = get_parameters_for_analysis(
        executable, file_type, extension, run_id, run_id_setup, input_directories, output_directory)

    print("\033[32m" + "All checks passed, running analysis now." + "\033[0m")

    pool = PoolThread(threads)
    if progress_bar:
        for _ in tqdm.tqdm(pool.imap(run_analysis, list_of_params), total=len(list_of_params)):
            pass
    else:
        pool.map(run_analysis, list_of_params)

    pool.close()
    pool.join()


def get_run_id_setup_mapping(run_id):
    run6_mapping = {"61": "6A",
                    "62": "6B",
                    "63": "6C",
                    "64": "6D"}

    if run_id[0] == "6":
        return run6_mapping[run_id]
    return run_id


def run_analysis(params):
    system(params)


def get_run_express_from_params(executable, file_type, filename, run_id, run_id_setup, output_directory):
    if output_directory:
        return "./{} -t {} -f {} -p conf_trb3.xml -u userParams.json -i {} -l detectorSetupRun{}.json -o {}".format(
            executable, file_type, filename, run_id, run_id_setup, output_directory)
    return "./{} -t {} -f {} -p conf_trb3.xml -u userParams.json -i {} -l detectorSetupRun{}.json".format(
        executable, file_type, filename, run_id, run_id_setup)


def get_parameters_for_analysis(executable, file_type, extension, run_id, run_id_setup, input_directories, output_directory):
    list_of_params = []
    for directory in input_directories:
        for fname in filter(listdir(directory), "*.{}".format(extension)):
            list_of_params.append(
                [executable, file_type, directory + fname, run_id, run_id_setup, output_directory])
    return [get_run_express_from_params(*x) for x in list_of_params]

if __name__ == "__main__":
    main()
