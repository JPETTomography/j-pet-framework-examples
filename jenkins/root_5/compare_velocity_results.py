#!/usr/bin/python
from __future__ import print_function
import csv
import sys


def parse_file(input_file, test_file, multiplier):
    input = open(input_file)
    test = open(test_file)
    for (input_line, test_line) in zip(input, test):
        input_splitted = input_line.rstrip().split("\t")
        test_splitted = test_line.rstrip().split("\t")
        print(input_splitted)
        value_diff = abs(
            (float)(input_splitted[1]) - (float)(test_splitted[1]))
        if (value_diff * (float)(multiplier)) <= abs((float)(input_splitted[2])) and (value_diff * (float)(multiplier)) <= abs((float)(test_splitted[2])):
            continue
        else:
            print("Value diff %f, inputvalue diff: %s, testvalue diff: %s" %
                  (value_diff * (float)(multiplier), input_splitted[2], test_splitted[2]))
            return 1

    return 0


sys.exit(parse_file(sys.argv[1], sys.argv[2], sys.argv[3]))
