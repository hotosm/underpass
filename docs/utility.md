# Utility Programs

## data/setupdb.sh

This is a simple shell script that creates all the databases Underpass
uses. It can also be used to initialize the two internal databases
Underpass uses, one for the *.state.txt files that contain the
timestamp of the data files, and the other is for the boundaries of
countries used to determine which country a change was made in for
statistics collection. These are used to bootstrap a new Underpass
installation.
