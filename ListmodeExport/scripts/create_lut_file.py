"""
This module parses PRIM geometry file that can be generated from GATE to CASToR binary LUT file.
Currently supports `scanner` and `cylindricalPET` scanner types.

As a input it takes path to PRIM file and it output binary LUT file and corresponding ASCII header file.
"""

import argparse
import sys
import numpy as np


class Scanner:

  def __init__(self, name, description):
    self.name = name
    self.description = description
    self.layers = {}
    self.crystals_size_x = None
    self.crystals_size_y = None
    self.crystals_size_z = None
    self.current_layer = None
    self.voxels_number = 161
    self.fov = 45
    self.depth_interaction = -1
    self.angle_diff = 90

  def set_crystals_size(self, x, y, z):

    self.crystals_size_x = x
    self.crystals_size_y = y
    self.crystals_size_z = z

  def add_crystal(self, position, orientation_vector, size):
    if size[2] > self.crystals_size_z:
      split_num = int((2 * size[2]) / self.crystals_size_z)
      position[2] -= self.crystals_size_z + (
          split_num / 2 * self.crystals_size_z
      )  # start at -width of crystal
      for _ in range(split_num):
        position[2] += self.crystals_size_z
        self.layers[self.current_layer]["data"] = np.append(
            self.layers[self.current_layer]["data"], position
        )
        self.layers[self.current_layer]["data"] = np.append(
            self.layers[self.current_layer]["data"], orientation_vector
        )
        self.layers[self.current_layer]["crystal_num"] += 1
    else:
      self.layers[self.current_layer]["data"] = np.append(
          self.layers[self.current_layer]["data"], position
      )
      self.layers[self.current_layer]["data"] = np.append(
          self.layers[self.current_layer]["data"], orientation_vector
      )
      self.layers[self.current_layer]["crystal_num"] += 1

  def change_layer(self, name):
    if name in self.layers:
      self.current_layer = name
    else:
      self.add_layer(name)

  def add_layer(self, name):
    self.layers[name] = {
        "data": np.array([], dtype=np.float32),
        "crystal_num": 0
    }
    self.current_layer = name

  def save_scanner(self, path):

    out_vector = np.array(
        [val['data'] for val in self.layers.values()], dtype=np.float32
    )
    with open(path + "/" + self.name + '.lut', 'wb') as out:
      out_vector.tofile(out)
    self.write_header(path)

  def write_header(self, out_path):

    layer_elements = []
    layer_crystals_size_x = []
    layer_crystals_size_y = []
    layer_crystals_size_z = []
    layer_mean_depth_interaction = []
    number_of_layers = len(self.layers.keys())
    #pylint: disable=consider-using-dict-items
    #here we explicite do not want to loop by key/value pair
    for key in self.layers:
      layer_elements.append(self.layers[key]["crystal_num"])
      layer_crystals_size_x.append(self.crystals_size_x)
      layer_crystals_size_y.append(self.crystals_size_y)
      layer_crystals_size_z.append(self.crystals_size_z)
      layer_mean_depth_interaction.append(self.depth_interaction)

    all_elements = sum(layer_elements)

    header = {
        "modality": "PET",
        "scanner name": self.name,
        "description": self.description,
        "number of elements": all_elements,
        "number of layers": number_of_layers,
        "voxels number transaxial": self.voxels_number,
        "voxels number axial": self.voxels_number,
        "field of view transaxial": self.fov,
        "field of view axial": self.fov,
        "number of crystals in layer": layer_elements,
        "crystals size axial": layer_crystals_size_x,
        "crystals size trans": layer_crystals_size_y,
        "crystals size depth": layer_crystals_size_z,
        #optional field
        "mean depth of interaction": layer_mean_depth_interaction,
        "min angle difference": self.angle_diff,
    }
    with open(out_path + "/" + self.name + '.hscan', 'w',
              encoding="utf-8") as out:
      for key, value in header.items():
        print(f"{key}: {value}".replace("[", "").replace("]", ""), file=out)


def handle_pvname(line):

  return line.replace("#/PVName", "").strip().split(".")


def handle_ndiv(line):

  return int(line.replace("/Ndiv", "").strip())


def handle_list(line, name):

  return np.array(
      ' '.join(line.replace(name, "").strip().split()).split(),
      dtype=np.float32
  )


def handle_origin(line):

  return handle_list(line, "/Origin")


def handle_base_vector(line):

  return handle_list(line, "/BaseVector")


def handle_box(line):

  return handle_list(line, "/Box")


def parse_segment(lines, scanner, crystal_pvname="crystal", skip_layer=None):
  name = None
  print_info = True
  while len(lines) > 0:
    line = lines.pop(0)
    if line.startswith("#/PVName"):
      name, name_id = handle_pvname(line)
    elif line.startswith("/Origin"):
      origin = handle_origin(line)
    elif line.startswith("/BaseVector"):
      base_vector = handle_base_vector(line)
    elif line.startswith("/Box"):
      box = handle_box(line)
    elif line.startswith("#--------------------"):
      break
    if name and name.startswith("module") and print_info:
      print(f"Parsing module {name_id}...")
      print_info = False

  if name and (skip_layer is None or skip_layer not in name):
    if crystal_pvname == "layer" and name.startswith("layer"):
      scanner.change_layer(name)
    elif crystal_pvname == "crystal" and name.startswith("world"):
      scanner.change_layer(name)

  if name and name.startswith(crystal_pvname):
    print(f"Adding crystal {name_id}({name})...")
    # PRIM format contains information about rotation transformed to x and y axis
    # we only want x axis base vector, so here we are selecting only 3 first values
    # from base_vector
    scanner.add_crystal(origin, base_vector[:3], box)


def parse_prim_file(args_parsed):
  """
  Main function used to parse PRIM file and generate LUT file with corresponding header.

  Args:
      scanner_name (str): name of generated scanner
      description (str): description of generated scanner
      filename (str): path to PRIM filename to parse
      crystals_size (:obj:`list` of :obj:`float`): size of crystals (x, y, z)
      save_path (str): path where generated LUT and corresponding header will be saved
      scanner_type (str): type of scanner in provided PRIM file,
                            currently supported values: `scanner` and `cylindricalpet`
      voxels_number (int): default number of voxels in reconstructed image
      fov (int): default fov value for scanner
      depth_interaction (float): default value for depth integration in scanner
      angle_diff (int): minimal difference between 2 hits in event
  """
  scanner_name = args_parsed.scanner_name
  description = args_parsed.scanner_description
  filename = args_parsed.prim_file
  crystals_size = args_parsed.crystals_size
  save_path = args_parsed.save_path
  scanner_type = args_parsed.scanner_type
  voxels_number = args_parsed.voxels_number
  fov = args_parsed.fov
  depth_interaction = args_parsed.depth_interaction
  angle_diff = args_parsed.angle_diff
  skip_layer = args_parsed.skip_layer

  crystal_pvname = "crystal"
  if scanner_type == "cylindricalpet":
    crystal_pvname = "layer"
  s = Scanner(scanner_name, description)
  s.set_crystals_size(crystals_size[0], crystals_size[1], crystals_size[2])
  s.voxels_number = voxels_number
  s.fov = fov
  s.depth_interaction = depth_interaction
  s.angle_diff = angle_diff
  with open(filename, encoding="utf-8") as f:
    lines = f.readlines()

  while len(lines) > 0:
    parse_segment(lines, s, crystal_pvname, skip_layer)

  s.save_scanner(save_path)


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
      description='Parse Gate PRIM file to generate castor LUT file.'
  )
  parser.add_argument(
      '--scanner_name', type=str, help='Name of generated scanner'
  )
  parser.add_argument(
      '--scanner_description',
      type=str,
      help='Description of generated scanner'
  )
  parser.add_argument('--prim_file', type=str, help='Path to input PRIM file')
  parser.add_argument(
      '--crystals_size',
      type=float,
      nargs=3,
      help='Size of crystals (x, y, z)'
  )
  parser.add_argument(
      '--save_path',
      type=str,
      default=".",
      help='Path to location where generated LUT file will be saved'
  )
  parser.add_argument(
      '--scanner_type',
      type=str,
      default="scanner",
      help=
      "Type of scanner from PRIM file, currently supports only 'scanner' and 'cylindricalpet' types"
  )
  parser.add_argument(
      '--voxels_number',
      type=int,
      default=161,
      help="""
      Default value of reconstructed voxels number for scanner,
      this parameter can be overrided on reconstruction step.
      """
  )
  parser.add_argument(
      '--fov',
      type=int,
      default=45,
      help=
      "Default value of fov for scanner, this parameter can be overrided on reconstruction step."
  )
  parser.add_argument(
      '--depth_interaction',
      type=float,
      default=-1,
      help=
      "Default value of depth integration for scanner, this parameter can be overrided on reconstruction step"
  )
  parser.add_argument(
      '--angle_diff',
      type=int,
      default=90,
      help="""
      Default minimal value for angle difference between 2 hits in event for scanner,
      this parameter can be overrided on reconstruction step.
      """
  )
  parser.add_argument(
      '--skip_layer',
      type=str,
      default=None,
      help="""
      Name of layer to skip. This value can be useful if geometry contains WLS layer that shouldn't be taken into account
      in CASToR geometry.
      """
  )

  args = parser.parse_args()

  if args.scanner_type not in ("scanner", "cylindricalpet"):
    print("Please select 'scanner' or 'cylindricalpet' as a scanner type")
    sys.exit(1)

  parse_prim_file(args)
