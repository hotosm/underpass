## Development

The following flags are suggested when running the configuration for building:

`../configure CXX="ccache g++" CXXFLAGS="-std=c++17 -g -O0" CPPFLAGS="-DTIMING_DEBUG"`

Also, `gdb` for debugging:

`apt-get install gdb`

And it's very useful to have an alias for `gdb` in your bash profile.
Just add the following line add the bottom of `~/.bashrc`:


`alias lg='libtool --mode=execute gdb'`

And you'll be able to run the debugger, for example:

`lg testsuite/libunderpass.all/yaml-test`

