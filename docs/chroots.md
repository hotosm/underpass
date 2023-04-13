# Building Packages  in a Chroot

To distribute binary packages for a variety of platforms of varying
versions, it's common to use a *chroot* to build in. This is to make
sure that a the package binary is linked against the right libraries,
which is required so the user doesn't have to build anything from
source. Unlike Docker or Virtualbox, a chroot uses the base running
kernel on host machine, but provides a different set of runtime
libraries and tools.

# Creating a Chroot

All platforms that use the *.deb* packaging format, like Debian and
Ubuntu, can use the *debbootstrap* program to create the
chroot. Debootstrap lets you pick the architect to support,
`--arch amd64` builds files for the X86_64 platform. after that the
version of the distribution is specified, followed by the directory to
install the files in, and finally the URL to download packages
from. Once done, change to the installation directory, and type `sudo
chroot .`. You'll now be in a minimal runtime environment. The first
thing I do is create a `/etc/debian_chroot` file containing the name
of the distribution. This puts the this value into the command prompt
for the shell, which is very useful when you have multiple chroots.

As the *chroot* program requires one to be root, a utility program
called *schroot* can be used to fake root. Once the initial chroot is
created, change to the directory containing the downloaded files, and
type these shell commands. Some of the programs used to build packages
need these, which aren't present in the chroot, as they have to be
shared from the host platform.

```
mount -t proc proc /proc
mount --rbind /sys sys/
mount --rbind /dev dev/
```

Once that initial setup is done, it's time to download more
packages. The default config file for a chroot is limited to just the
main repository. Not all of the packages Underpass depends on are in
main, so add *universe* to get the rest, and then `apt-get
update`. Your `/etc/apt/sources.list` file should now look like this:

```
deb http://archive.ubuntu.com/ubuntu focal main universe
```

Finally edit  `~/.bashrc` and add these three lines at the bottom. One
specifies the path to a file pkg-config needs, but isn't always
distributed, so it's included. The other is for *locale*, so it stops
clutterin the terminal with warnings.

```
export PKG_CONFIG_PATH=/home/rob/underpass/m4
export LANG=C
export GPG_TTY="$(tty)"
```

To create signed packages, you need a *GPG* key pair setup, so might
as well do that now. Type `gpg --full-generate-key`, and answer the
questions. This will only work if you've execute the mount commands as
documented above. 

## For Debian

```
sudo /usr/sbin/debootstrap --arch amd64 bullseye bullseye/ http://deb.debian.org/debian/
```

Start by installing packages needed to build Underpass.

```
sudo apt-get -y install \
    git gcc g++ pkg-config make debhelper debconf chrpath ccache devscripts gpg \
    libgdal-dev libosmium2-dev libpq-dev libgumbo-dev \
    libssl-dev libpq5-dev libxml++2.6-dev libboost-all-dev
```

## Ubuntu Focal (20.04 LTS)

```
sudo /usr/sbin/debootstrap --arch amd64 focal focal/ http://archive.ubuntu.com/ubuntu/
```

Start by installing packages needed to build Underpass.

```
sudo apt-get -y install \
    git make gcc g++ pkg-config gpg debhelper debconf devscripts python3-all \
    libgdal-dev libpq-dev libgumbo-dev openmpi-bin \
    libxml++2.6-dev ccache libssl-dev libzip-dev libbz2-dev \
    libboost-all-dev libosmium2-dev osmium-tool \
    doxygen librange-v3-dev libtool-bin libltdl-dev  

```

## Ubuntu Groovy (20.10)

```
sudo /usr/sbin/debootstrap --arch amd64 groovy groovy/ http://archive.ubuntu.com/ubuntu/
```

Install dependencies as above. Note that the version of boost is 1.74

## Ubuntu Hirsute (21.04)

```
sudo /usr/sbin/debootstrap --arch amd64 hirsute hirsute/ http://archive.ubuntu.com/ubuntu/
```

Install dependencies as above. Note that the version of boost is 1.74

# Building Libpqxx Packages

The version of *libpqxx (6.x)* that's included in current (Aug 2021)
distribution like *Debian Bullseye* or *Unbuntu Hirsute (21.04)* has a bug triggered
by *libxml++*. Since Underpass uses libxml++ and libpqxx, we have to
build a package of a newer version (7.x). I added Debian packaging
files to a git repository to make this easier, Get that fork here:

```
git clone https://github.com/robsavoye/libpqxx.git
```

Once I do that, I run `git tag`, and the checkout the latest official
release branch. There's a configure bug in the libpqxx sources I
haven't gotten around to fixing yet, so I edit `configure.ac`, and
comment out this one line like so:

```
dnl AX_CXX_COMPILE_STDCXX_17([noext])
```

Then run `./autogen.sh`, which produces the scripts used for
configuring. Now that you have the `configure` script, configure
libpqxx like this:

```
./configure CXX="ccache g++" CXXFLAGS="-std=c++17 -g -O2" --disable-dependency-tracking
```

After that, change to the *debian* directory, and type `make deb -i
-k`. That'll build the deb packages, and at the end, have you GPG sign
them. Once built, install the two packages, and you're all set to go
build Underpass.

# Building Underpass Packages

The Underpass git repository is at:

https://github.com/hotosm/underpass.git

Once downloaded, change to the source directory and run
`./autogen.sh`. Once the configure files have been built, configure
Underpass like this:

```
./configure CXX="ccache g++" CXXFLAGS="-std=c++17 -g -O2" --disable-dependency-tracking
```

and then typing `make deb -i -k` builds the packages, and has you sign them.
