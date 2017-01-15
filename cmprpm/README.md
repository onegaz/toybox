This tool is intended to check content of two rpm files, see if they are the same.
E.g. one rpm is version 1.0_RC2.rpm, another rpm is 1.0.rpm. The version string is
part of path name. This tool ignore the path difference, only compare
file contents.

WARNING: This tool remove folders under current working directory.
WARNING: This tool ignore empty folders.
WARNING: This tool likely ignore symbolic links.
WARNING: This tool only check existence of file, does not care their physical location and count.
WARNING: this tool does not complain occurrence of the same file mismatch. E.g. one rpm install fileA at two location, another rpm install fileA at three locations.
WARNING: This tool does not care file names. only compare their content (SHA1 digest)
WARNING: This tool is not heavily tested. It may work with Python 2.7.5. Use it at your own risk!!!

This tool create two folders under currect working directory to store files extracted from two rpms:
$PWD/rpm1
$PWD/rpm2

if above folders exist, this tool will remove it and recreate them in order to make sure they are clean.
Please make sure you have write permission in current working directory.
