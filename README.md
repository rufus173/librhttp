
# Purpose

This is designed to be uses as a shared library, or can be used as a standalone executable to make http requests. Follow compilation instructions below to make the different options

# How it works

The library works with pointers to structs where all the data about connections, requests, and responses.
Working with these structs is made easier with specialised functions to initialise, pack and free these. It is advised to use these functions, as they have been tested and are known to work. It is also possible to manualy fill out and read from, but this can be harder.

# Compilation instructions

for compilation of the shared library, `cd src` then `make librufushttp.so`
for rufus-requests, the standalone library, `cd src` then `make rufus-requests`
