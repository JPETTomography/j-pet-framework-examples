#!/usr/bin/python
from __future__ import print_function
import csv
import sys


def compare_lines(first_line, second_line, multiplier):
    value_diff = abs(
        (float)(first_line[4]) - (float)(second_line[4]))
    if((int)(first_line[0]) == (int)(second_line[0])):
        if((int)(first_line[1]) == (int)(second_line[1])):
            if((str)(first_line[2]) == (str)(second_line[2])):
                if((int)(first_line[3]) == (int)(second_line[3])):
                    if((value_diff * (float)(multiplier)) <= abs((float)(first_line[5])) and ((value_diff * (float)(multiplier)) <= abs((float)(second_line[5])))):
                        return True
                    else:
                        print("Value diff %f, inputvalue diff: %s, testvalue diff: %s" %
                              (value_diff * (float)(multiplier), first_line[4], second_line[4]))
    return False


def parse_file(input_file, test_file, multiplier):
    input = open(input_file)
    test = open(test_file)
    for (input_line, test_line) in zip(input, test):
        input_splitted = input_line.rstrip().split("\t")
        test_splitted = test_line.rstrip().split("\t")
        if (compare_lines(input_splitted, test_splitted, multiplier)):
            continue
        else:
            return 1

    return 0


sys.exit(parse_file(sys.argv[1], sys.argv[2], sys.argv[3]))
