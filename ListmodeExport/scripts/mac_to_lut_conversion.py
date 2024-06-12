"""
Conversion from a GATE geometry macro to a CASToR LUT file.

This script supposes that CASToR utilities are available in $PATH.

Detailed usage information is available by running
    $ python mac_to_lut_conversion.py -h
"""

import argparse
import subprocess
import tempfile
import sys
import os
import re
import shutil
import datetime
import logging
import numpy as np
import pandas as pd

LOGGER = logging.getLogger(__name__)


def get_castor_config_path():
  """Retrieve the config path of current CASToR installation."""
  try:
    with subprocess.Popen("castor-config", stdout=subprocess.PIPE) as process:
      return process.stdout.read().decode("utf-8").rstrip()
  except FileNotFoundError:
    # The executable castor-printConfigPath does not exist
    return None


def backup_geom_if_needed(castor_config_scanner, castor_geom_filename):
  """Create a backup of a .geom file if it already exists."""
  castor_geom_path = os.path.join(castor_config_scanner, castor_geom_filename)
  if os.path.exists(castor_geom_path):
    timestamp = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
    backup_castor_geom_filename = castor_geom_filename + ".backup" + timestamp
    backup_castor_geom_path = os.path.join(
        castor_config_scanner, backup_castor_geom_filename
    )
    shutil.move(castor_geom_path, backup_castor_geom_path)
    return True
  return False


def gate_mac_to_geom(m, o):
  """Convert GATE macro to CASToR LUT file using CASToR utility."""
  error = subprocess.call(["castor-GATEMacToGeom", "-m", m, "-o", o])
  return error == 0


# ----- CASToR transformation tools -----
CASTOR_DECIMAL_NUMBER_PATTERN = "[+-]?(?:[0-9]*[.])?[0-9]+"


def read_gate_macro(gate_mac_path, gate_macro):
  """Parse a GATE .mac file and extract a macro as an array."""
  with open(gate_mac_path, encoding="utf-8") as gate_mac_file:
    gate_mac_content = gate_mac_file.read()
    mac_regexp_result = re.search(
        rf"{gate_macro}((?:[\t ]+\S+)+)", gate_mac_content
    )
    if mac_regexp_result is None:
      # GATE macro not found
      return None
    macro_arguments = mac_regexp_result.group(1).split()
    return macro_arguments


def extract_castor_field(castor_geom_content, field_name):
  """Parse a CASToR .geom file content and extract a field as an array."""
  field_regexp_result = re.search(
      rf"{field_name}\s*:\s*((?:{CASTOR_DECIMAL_NUMBER_PATTERN},?)+)",
      castor_geom_content
  )
  field_array = field_regexp_result.group(1).split(",")
  return [float(r) for r in field_array]


def transform_castor_field(castor_geom_content, field_name, f):
  """Apply function f to each element of a given field of a CASToR .geom file content."""
  field_array = extract_castor_field(castor_geom_content, field_name)
  field_array_converted = [str(f(int(x))) for x in field_array]
  return re.sub(
      rf"({field_name}\s*:\s*)((?:{CASTOR_DECIMAL_NUMBER_PATTERN},?)+)",
      r"\g<1>" + ",".join(field_array_converted), castor_geom_content
  )


def fix_castor_geom(castor_geom_path, gate_mac_path, ring_number, ring_gap):
  """Fix CASToR .geom file that is sometimes wrongly converted by CASToR.
  This function assumes that the gap is constant between each pair of ring.
  """
  with open(castor_geom_path, "r+", encoding="utf-8") as castor_geom_file:
    # First, look for field "number of rsectors axial" and check values
    castor_geom_content = castor_geom_file.read()
    rsectors_array = extract_castor_field(
        castor_geom_content, "number of rsectors axial"
    )

    if any(r != 1 for r in rsectors_array):
      # In this case, fixing the CASToR .geom file is not obvious
      return False

    if not all(r == ring_number for r in rsectors_array):
      # At this point some fields of the CASToR .geom file must be converted
      castor_geom_content = transform_castor_field(
          castor_geom_content, "number of rsectors axial",
          lambda x: ring_number
      )
      castor_geom_content = transform_castor_field(
          castor_geom_content, "rsector gap axial", lambda x: ring_gap
      )
      # We assume here that "number of crystals axial" will always be a multiple of ring_number
      castor_geom_content = transform_castor_field(
          castor_geom_content, "number of crystals axial",
          lambda x: x // ring_number
      )

    # The rsectors rotation angle is not picked up by CASToR, hence the need of a manual fix
    gate_macro_value = read_gate_macro(
        gate_mac_path, "/gate/cylindricalPET/placement/setRotationAngle"
    )
    if gate_macro_value is not None:
      # There is a rotation that needs to be accounted for
      rotation_angle = float(gate_macro_value[0])
      castor_geom_content = transform_castor_field(
          castor_geom_content, "rsectors first angle",
          lambda x: x + float(rotation_angle)
      )

    # Finally, update the file itself
    castor_geom_file.seek(0)
    castor_geom_file.write(castor_geom_content)

    return True


# ---------------------------------------


def scanner_lut_explorer(m, o):
  """Dump crystals from CASToR .geom file to text file using CASToR utility."""
  with open(o, "w", encoding="utf-8") as output_file:
    error = subprocess.call(
        ["castor-scannerLUTExplorer", "-sf", m, "-g"], stdout=output_file
    )
    return error == 0
  return False


def clean_dump_file(dump_path):
  """Clean dump file to keep only lines starting with 'Scanner element center location...'."""
  with open(dump_path, "r", encoding="utf-8") as dump_file:
    lines = dump_file.readlines()
  with open(dump_path, "w", encoding="utf-8") as dump_file:
    for line in lines:
      if line.startswith("Scanner element center location"):
        dump_file.write(line)


def prepare_lut(dump_file, log_file, castor_lut):
  """Prepare CASToR LUT from crystal dump."""

  # (function originally written by Jakub Baran)

  f_in = dump_file
  f_out = log_file

  with open(f_out, "w", encoding="utf-8") as ff_out:
    with open(f_in, "r", encoding="utf-8") as ff_in:
      for line in ff_in:
        s_line = re.split(": | ; | .O", line.rstrip())
        ff_out.write(
            "\t".join(
                [
                    s_line[1], s_line[2], s_line[3], s_line[5], s_line[6],
                    s_line[7]
                ]
            ) + "\n"
        )

  data = pd.read_csv(f_out, delimiter="\t", header=None)

  a = np.array(data).flatten()
  with open(castor_lut, "wb") as output_file:
    float_array = np.array(a, dtype=np.float32)
    float_array.tofile(output_file)
    output_file.close()


# ----- Argparse types -----
def ring_number_type(number):
  """Check whether a number can represent a number of ring."""
  number = int(number)
  if number <= 0:
    raise argparse.ArgumentTypeError("The number of ring must be > 0.")
  return number


def gap_size_type(number):
  """Check whether a number can represent a gap size."""
  number = int(number)
  if number < 0:
    raise argparse.ArgumentTypeError("The gap size must be ≥ 0.")
  return number


# ----------------------------------


def main():
  parser = argparse.ArgumentParser(
      description="Conversion from a GATE geometry macro to a CASToR LUT file."
  )

  parser.add_argument(
      "-m", "--gate_macro", help="Input GATE geometry macro", required=True
  )
  parser.add_argument(
      "-l", "--castor_lut", help="Output CASToR LUT file", required=True
  )

  # If not provided, this script tries to automatically determine CASToR config path
  parser.add_argument(
      "-c",
      "--castor_config",
      help="Path to CASToR config folder",
      required=False
  )

  parser.add_argument(
      "--keep_dump_file", help="Keep the dump file", action="store_true"
  )
  parser.add_argument(
      "--keep_log_file", help="Keep the log file", action="store_true"
  )

  parser.add_argument(
      "-v",
      "--verbose",
      action="store_true",
      help="Display additional information",
      required=False
  )

  # CASToR .geom file correction
  geom_correction_group = parser.add_argument_group(
      "CASToR .geom file correction"
  )
  geom_correction_group.add_argument(
      "--ring_number",
      help="Number of rings",
      type=ring_number_type,
      required=True
  )
  geom_correction_group.add_argument(
      "--ring_gap", help="Gap between rings", type=gap_size_type
  )

  args = parser.parse_args()

  if args.ring_number > 1 and args.ring_gap is None:
    parser.error(
        "--ring_gap must be specified when there are more than one ring."
    )

  gate_macro = args.gate_macro
  castor_lut = args.castor_lut
  castor_config = args.castor_config
  keep_dump_file = args.keep_dump_file
  keep_log_file = args.keep_log_file
  verbose = args.verbose
  ring_number = args.ring_number
  ring_gap = args.ring_gap

  if verbose:
    logging.basicConfig(level=logging.INFO)
  else:
    logging.basicConfig(level=logging.WARNING)

  if castor_config is None:
    castor_config = get_castor_config_path()
    if castor_config is None:
      sys.exit(
          "Cannot automatically retrieve CASToR config. Please use --castor_config."
      )
    LOGGER.info(f"CASToR config path found in {castor_config}.")

  castor_geom_basename = "castor_geom"
  castor_geom_filename = castor_geom_basename + ".geom"  # if that geometry exists, it will be erased!
  castor_config_scanner = os.path.join(castor_config, "scanner")
  castor_geom_path = os.path.join(castor_config_scanner, castor_geom_filename)

  if backup_geom_if_needed(castor_config_scanner, castor_geom_filename):
    LOGGER.info("Previous .geom file backup created.")

  dump_filename = "dump.txt"
  log_filename = "prepare_lut.log"

  with tempfile.TemporaryDirectory() as tmp_dir:

    LOGGER.info(f"Working in temporary directory {tmp_dir}.")

    # 1. Convert GATE macro to CASToR .geom file
    if not gate_mac_to_geom(gate_macro, castor_geom_basename):
      sys.exit("Conversion from GATE macro to CASToR .geom file failed.")
    LOGGER.info("GATE macro successfully converted to CASToR .geom file.")

    # Fix CASToR .geom file with respect to number of rings
    if not fix_castor_geom(castor_geom_path, gate_macro, ring_number,
                           ring_gap):
      sys.exit("Cannot fix the CASToR .geom file.")
    LOGGER.info("CASToR .geom file successfully fixed, or nothing to fix.")

    # 2. List all crystals from CASToR .geom file to .txt dump
    dump_path = os.path.join(tmp_dir, dump_filename)
    if not scanner_lut_explorer(castor_geom_path, dump_path):
      sys.exit("List crystals from CASToR .geom file failed.")
    LOGGER.info("Crystals from CASToR .geom file successfully listed.")

    # Keep the dump file if asked by the user
    if keep_dump_file:
      shutil.copy(dump_path, ".")

    # 3. Remove from dump.txt useless lines
    clean_dump_file(dump_path)
    LOGGER.info("Dump file cleaned.")

    # 4. Prepare LUT from dump
    if keep_log_file:
      log_path = log_filename
      LOGGER.info(f"Log file will be written in {log_path}.")
    else:
      log_path = os.path.join(tmp_dir, log_filename)
    prepare_lut(dump_path, log_path, castor_lut)
    LOGGER.info(f"CASToR LUT file successfully written in {castor_lut}.")

    # 5. (Optional) Clean the CASToR .geom file
    os.remove(castor_geom_path)  # do we want to keep it?


if __name__ == "__main__":
  main()
