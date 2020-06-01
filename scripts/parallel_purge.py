#!/usr/bin/env python3
from fnmatch import filter
from termcolor import colored
import tqdm
import os
import sys
import subprocess

import argparse

from multiprocessing.dummy import Pool as PoolThread

parser = argparse.ArgumentParser(
    "Python script to purge root files in parallel")
parser.add_argument("directory", type=str,

                    help="Directory in which you want to purge files.")
parser.add_argument("-n", "--number-of-threads", required=False, default=20, type=int,
                    help="Number of threads to run simultaneously")

args = vars(parser.parse_args())
# root macro run format
# root -l "macro.C+(arg1,arg2,arg3,...,argn)"


def argConversion(arg):
    if type(arg) in [float, int]:
        return str(arg)
    elif type(arg) in [str]:
        return '"' + arg + '"'
    else:
        return '"' + str(arg) + '"'


def argConnection(arglist=[]):
    if not (arglist is [] or arglist is None):
        shellargv = list(arglist)
        arglist_conv = list()
        for arg in arglist:
            arglist_conv.append(argConversion(arg))
        return '(' + ','.join(arglist_conv) + ')'
    else:
        return ''


def runMacro(macroName, arglist=None, splash=False, interprete=False, batch=True):
    shellCommand = ['root']
    if interprete is False:
        shellCommand.append("-q")
    if splash is False:
        shellCommand.append("-l")
    if batch is True:
        shellCommand.append("-b")
    shellCommand.append(macroName + argConnection(arglist))
    print("Run Macro", shellCommand)
    a = subprocess.Popen(shellCommand)
    return a


def run_macro_parallel(file):
    runMacro('purge.C', arglist=[file])


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

    if not os.path.isdir(input_directory):
        print(colored(
            "Specified input drectory des not exist. Please check spelling.", "red"))
        exit()

    list_of_files = filter(os.listdir(input_directory), "*.root")

    pool = PoolThread(threads)

    if progress_bar:
        for _ in tqdm.tqdm(pool.imap(run_macro_parallel, list_of_files), total=len(list_of_files)):
            pass
    else:
        result = pool.map(run_macro_parallel, list_of_files)


if __name__ == "__main__":
    main()
