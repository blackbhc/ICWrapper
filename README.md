########## The opencode projects used in my pipeline
#
###
HighFive: git@github.com:BlueBrain/HighFive.git
Header only library for IO of hdf5 files, It's much more user and beginner friendly.
Note this library should work with standard hdf5 library.
#
###
mINI: git@github.com:pulzed/mINI.git
Header only library, .ini file parameter parsers
#
###
boost 1.81.0: https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/
Supplementary library of C++, which is also headre only.



########## Usage of binary executables:
#
###
ICconvert: specify the transform parameter ini files, and them run "bin/ICconvert <dirctory of inifile>"
A template of such IC format transform ini file is available in the directory ./ini, named as ICformat.ini-Template


########## Erorr return convention:
#
###
100: file not found
101: out of memory
102: parameter inconsistence
