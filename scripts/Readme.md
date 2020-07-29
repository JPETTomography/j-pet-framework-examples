# Scripts for running the J-PET Framework analyses in parallel

## parallel_analysis.py

This script is used to run parallel analysis of J-PET Framework example in easy way. It takes directory or list of directories as an input and analyzes all root files in them.

### Usage:

```
./parallel_analysis.py <exacutable> [-i | --input] <directory_to_analyze> [-r | --run_id] <id_of_run>
```
User can specify a single directory, list of directories (ex. dir1 dir2 dir3), or the glob expression (dir*, or dir/*/dir)

### Additional options
To see all possible options run:
```
./parallel_analysis.py [-h | --help]
```
User can specify output directory with a flag:
```
[-o | --output] <output_directory> (default: same as input)
```
analyse only files with specific extension:
```
[-e | --extension] <extension>  (default: root)
```
analyze different type of file:
```
[-t | --type] <type> (default: root)
```
disable progress bar:
```
[-p | --progress-bar]
```
or change default number of processes to run simultaneously:
```
[-n  | --number_of_threads] (default: 20)
```

## purge.C

ROOT script to remove all ROOT file contents except for TDirectory-based objects.

In practice, when applied to a file produced by hadd-ing multiple output files from the J-PET Analysis Framework, it will remove anything besides directories with histograms (especially multiple instances of ParamBank, which are a common nuisance with hadd-ed files, will be deleted) allowing for faster opening of files and inspection of histograms.

### Usage:

```sh
root "purge.C(\"path_to_hadded_root_file.root\")"
```

The script modifies the indicated file in-place


## parallel_purge.py

Python script to run purge.C in parallel to, i.e. purge files before hadding. Script "purge.C" must be located in the same directory as parallel_purge.py

### Usage:
```
./parallel_purge.py <directory_to_purge>
```

### Additional options
To see all possible options run:
```
./parallel_purge.py [-h | --help]
```

You can disable progress bar by running:
```
[-p | --progres_bar]
```
and set number of parallel proceses by:
```
[-n | --number_of_threads] (default: 20)
```
