# Pharos Visual C++ Demangler

The Pharos Visual C++ Demangler is a symbol de-mangling tool that
demangles C++ symbol named in order to retrieve the original C++
declaration.  It also includes a JSON output mode which splits the
symbols up into their constituent parts for type analysis, etc.  It
exists both as a standalone program and as a library which linked to
by other programs.

## Build and Install

`cmake` is the build tool used for this project.  The build has only
been tested under Linux, but should be easily adaptable to other
systems if needed.

From the top directory of the distribution:
```
mkdir build
cd build
cmake ..
make
make install
```

## Known deficiencies

- C++/CLI symbols are very poorly supported due to both lack of
  references and lack of a reasonable test corpus

## State of the code

This code was written to support our own research goals.  It was
hacked together in a short amount of time, and things were added when
needed in a very ad-hoc fashion.  Although some time was spent
cleaning up code for release, do not expect this to be the cleanest
bit of code in the world.

## References

The information we used to create this program came from three primary sources:

- Wikiversity [Visual C++ name
  mangling](https://en.wikiversity.org/wiki/Visual_C%2B%2B_name_mangling)
- [Calling
  conventions](http://www.agner.org/optimize/calling_conventions.pdf)
  by Agner Fog
- Output from the Microsoft Visual C++ compiler and its `undname.exe`
  demangler


## License

Pharos Demangler

Copyright 2017 Carnegie Mellon University. All Rights Reserved.

BSD

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the
   distribution.

This material is based upon work funded and supported by the
Department of Defense under Contract No. FA8702-15-D-0002 with
Carnegie Mellon University for the operation of the Software
Engineering Institute, a federally funded research and development
center.

The view, opinions, and/or findings contained in this material are
those of the author(s) and should not be construed as an official
Government position, policy, or decision, unless designated by other
documentation.

NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. CARNEGIE MELLON
UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR
IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF
FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS
OBTAINED FROM USE OF THE MATERIAL. CARNEGIE MELLON UNIVERSITY DOES NOT
MAKE ANY WARRANTY OF ANY KIND WITH RESPECT TO FREEDOM FROM PATENT,
TRADEMARK, OR COPYRIGHT INFRINGEMENT.

[DISTRIBUTION STATEMENT A] This material has been approved for public
release and unlimited distribution.  Please see Copyright notice for
non-US Government use and distribution.
