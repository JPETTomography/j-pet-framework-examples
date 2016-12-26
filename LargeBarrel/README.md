# LargeBarrel

This code is intended to be a part of [J-PET Framework analysis examples](https://github.com/JPETTomography/j-pet-framework-examples).

The purpose of this example is to have standarized way of constructing different data structures (signals, hits, etc.) and is meant to be incorporated into J-PET Framework examples code.

To install this example just clone this repository in a j-pet-framework-examples directory and append that submodule into *CMakeLists.txt* file

```
cd j-pet-framework-examples/
git clone https://github.com/Alvarness/LargeBarrel
echo "add_subdirectory(LargeBarrel)" >> CMakeLists.txt
```

and then compile as usual.
