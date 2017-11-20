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

## JSON output

There are currently two JSON formats emitted by the command-line
tool.  Raw and minimal.

The minimal format is only really useful for functions and methods, and
outputs the full type names for the argument and return value of these
functions.  For other symbol types, it will duplicate the raw format.

The raw format was originally written to help debug the demangler.  It
is a direct JSON representation of the C++ structure that represents
the demangled name.

A user-friendly non-minimal JSON output schema has been worked on, but
has not been completed.  (This is what `--json` without qualifiers is
reserved for as a command-line argument.)  Figuring out a happy medium
between the raw format and the minimal format is harder than one might
think.

## Known deficiencies

- C++/CLI symbols are very poorly supported due to both lack of
  references and lack of a reasonable test corpus

## undname differences

When comparing output to *undname.exe*, the pharos demangler differs
in some important places:

- Spacing may be different.
- *undname.exe* sometimes omits `const` in places where that `const` is
  definitely encoded in the symbol.  We've been able to determine that
  constant pointers have the const elided when used as a return value.
  Other circumstances exists, though, that we have not been able to
  figure out.
- In most places *undname.exe* outputs anonymous namespaces as
  `` `anonymous namespace'``, but sometimes it fails and will output the
  literal string in the mangled name representing the namespace, such
  as ``A0xcfd685c9``.
- There are some mangled names that *undname.exe* just fails
  completely on that the Pharos demangler either succeeds on or "does
  better."  Any undname result with a `?` or a `@` in it is considered
  a failed result.

## State of the code

This code was written to support our own research goals.  It was
hacked together in a short amount of time, and things were added when
needed in a very ad-hoc fashion.  Although some time was spent
cleaning up code for release, do not expect this to be the cleanest
bit of code in the world.  In particular, the `DemangledType` data
structure has accumulated members at need.

## References

The information we used to create this program came from three primary sources:

- Wikiversity [Visual C++ name
  mangling](https://en.wikiversity.org/wiki/Visual_C%2B%2B_name_mangling)
- [Calling
  conventions](http://www.agner.org/optimize/calling_conventions.pdf)
  by Agner Fog
- Output from the Microsoft Visual C++ compiler and its `undname.exe`
  demangler


## Legal

Pharos Demangler

Copyright 2017 Carnegie Mellon University. All Rights Reserved.

NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. CARNEGIE MELLON
UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR
IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF
FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS
OBTAINED FROM USE OF THE MATERIAL. CARNEGIE MELLON UNIVERSITY DOES NOT
MAKE ANY WARRANTY OF ANY KIND WITH RESPECT TO FREEDOM FROM PATENT,
TRADEMARK, OR COPYRIGHT INFRINGEMENT.

Released under a BSD-style license, please see license.txt or contact
permission@sei.cmu.edu for full terms.

[DISTRIBUTION STATEMENT A] This material has been approved for public
release and unlimited distribution.  Please see Copyright notice for
non-US Government use and distribution.

DM17-0949
