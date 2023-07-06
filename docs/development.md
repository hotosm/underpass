## Development and debugging

The following flags are suggested when running the configuration for building:

`../configure CXX="ccache g++" CXXFLAGS="-std=c++17 -g -O0" CPPFLAGS="-DTIMING_DEBUG"`

Also, `gdb` for debugging:

`apt-get install gdb`

And it's very useful to have an alias for `gdb` in your bash profile.
Just add the following line add the bottom of `~/.bashrc`:

`alias lg='libtool --mode=execute gdb'`

And you'll be able to run the debugger, for example:

`lg src/testsuite/libunderpass.all/yaml-test`

## Debugging in MacOS

It's also possible to debug Underpass on MacOS. You should use `glibtool` instead of `libtool`.

`brew install gdb`

Note that gdb requires special privileges to access Mach ports.
You will need to codesign the binary. For instructions, see: https://sourceware.org/gdb/wiki/PermissionsDarwin

Add the alias in your `~/.bashrc` file:

`alias lg='glibtool --mode=execute gdb'`

