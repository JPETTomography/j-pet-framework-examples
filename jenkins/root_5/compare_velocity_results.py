#!/usr/bin/python
from __future__ import print_function
import csv
import sys

def compare_lines(first_line, second_line):
    value_diff = abs(
            (float)(first_line[4]) - (float)(second_line[4]))
    return first_line[0] == second_line[0] and first_line[1] == second_line[1] and first_line[2] == second_line[2] and first_line[3] == second_line[3] and first_line[6] == second_line[6] and first_line[7] == second_line[7] and first_line[8] == second_line[8] and first_line[9] == second_line[9] and first_line[10] == second_line[10] and first_line[11] == second_line[11] and value_diff * (float)(multiplier)) <= abs((float)(input_splitted[5])) and (value_diff * (float)(multiplier)) <= abs((float)(test_splitted[5]) 


def parse_file(input_file, test_file, multiplier):
    input = open(input_file)
    test = open(test_file)
    for (input_line, test_line) in zip(input, test):
        input_splitted = input_line.rstrip().split("\t")
        test_splitted = test_line.rstrip().split("\t")
        if (compare_lines(input_line, test_line)):
            continue
        else:
            print("Value diff %f, inputvalue diff: %s, testvalue diff: %s" %
                  (value_diff * (float)(multiplier), input_splitted[2], test_splitted[2]))
            return 1

    return 0


sys.exit(parse_file(sys.argv[1], sys.argv[2], sys.argv[3]))
