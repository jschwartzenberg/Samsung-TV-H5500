#!/bin/bash

check_var () {
        if [[ -z "$1" ]]; then
                echo "Error: not defined first parameter for check_var() function:"
                echo "variable name"
                exit 1
        fi
        eval VARIABLE_VALUE="\$$1"

        if [ -z "$2" ]; then
                # Global has no description
                if [ -z "$VARIABLE_VALUE" ]; then
                        echo "Error: please specify $1 variable"
                        exit 1
                fi
        else
                # Additionally describe variable
                if [ -z "$VARIABLE_VALUE" ]; then
                        echo "Error: please specify $1 variable: $2"
                        exit 1
                fi
        fi
}

check_dir () {
        if [[ -z "$1" ]]; then
                echo "Error: not defined first parameter for check_dir() function:"
                echo "directory name"
                exit 1
        fi

        if [[ ! -d "$1" ]]; then
                echo "Error: can not find $1 directory"
                exit 1
        fi
}

check_file () {
        if [[ -z "$1" ]]; then
                echo "Error: not defined first parameter for check_file() function:"
                echo "file name"
                exit 1
        fi

        if [[ ! -f "$1" ]]; then
                echo "Error: can not find $1 file"
                exit 1
        fi
}

#parameters
# $1 Source directory name
# $2 Destination directory name
copy_dir_clean () {

	if [ -d "$2" ]; then

		echo "copy directory $1 to $2"

		rm -rf $2/*

	        pushd $1
			tar cf - --exclude=CVS --exclude=.svn --exclude=.git --exclude="*.orig" --exclude="*.rej" * | ( cd $2 && tar xvpf - )
	        popd
	fi
}

### Default variables. ###
PKG_VERSION="VDLPv4"

### Script internal variables. ###
pushd `dirname $0` > /dev/null
TOP=`pwd`
popd > /dev/null

CONFIG_FILE=""

DIST=${TOP}/glibc
SRC=${TOP}/glibc-src
INSTALL=${TOP}/glibc-install
BUILD=${TOP}/glibc-build

### Helper functions. ###
abort ()
{
  echo -e "$1\n"
  exec false
}

check_success ()
{
  if [ $? -ne 0 ]
  then
    abort "$1 ..  Failed"
    exit 1
  fi
  echo "$1 .. Done"
}

help () {
  echo "Usage: $0 [-d DIST_DIR] [-s SRC_DIR] [-b BUILD_DIR] [-i INSTALL_DIR]"
  echo "Build the GLIBC library using the cross toolchain."
  echo
  echo "  -c set configuration file. Availible configs:"
  echo "`ls ${TOP}/toolchain.cfg/*.config | sed -e "s/^.*\///" | sed -e "s/^/      /"`"
  echo "  -d set glibc distribution directory (by default $DIST will be used)" 
  echo "  -s set directory to put prepared sources (by default $SRC will be used)"
  echo "  -b set build directory (by default $BUILD will be used)"
  echo "  -i set install directory (by default $INSTALL will be used)"
  echo "  -r remove SRC_DIR, BUILD_DIR and BUILD_DIR"
  echo "  -h print this message"
  echo
}

### Parse command line options. ###
while getopts "c:d:s:b:i:rh" OPTION
do
  case $OPTION in
    c)
      CONFIG_FILE=${TOP}/toolchain.cfg/"$OPTARG"
      ;;
    d)
      DIST="$OPTARG"
      ;;
    s)
      SRC="$OPTARG"
      ;;
    b)
      BUILD="$OPTARG"
      ;;
    i)
      INSTALL="$OPTARG"
      ;;
    r)
      rm -rf $SRC $BUILD $INSTALL
      exit 0
      ;;
    h)
      help
      exit 0
      ;;
    *)
      echo "Error. Unknown option or missed argument."
      echo "To get help run $0 -h"
      exit -1
  esac
done

### Checking config-file. ###
if [[ -z "${CONFIG_FILE}" ]]; then
	echo "Please specify config file"
	help
	exit 0
fi

### Including of configuration file. ###
source $CONFIG_FILE || abort "Can not source $CONFIG_FILE as configuration file"

PRODUCT_VERSION=`echo $CONFIG_FILE | sed -e "s/^.*-\(v[0-9]*\)\.config/\1/"`

check_var "TARGET" "target triplet"
check_var "WITH_FLOAT" "float pointing mode"
check_var "PRODUCT_VERSION" "product version"

BUILD_TARGET=${TARGET}
if [[ "$WITH_FLOAT" == "soft" ]]; then
	FP="without-fp"
else
	FP="with-fp"
fi
CROSS_GCC=${CROSS_GCC:-${BUILD_TARGET}-gcc}

### Check the cross-compiler. ###
$CROSS_GCC -v 2> /dev/null
check_success "Checking the cross-compiler in \$PATH"

## Make absolute paths. ###
pushd $DIST > /dev/null
DIST=`pwd`
popd > /dev/null
check_success "Checking the glibc distribution directory"

mkdir -p $SRC
pushd $SRC > /dev/null
SRC=`pwd`
popd > /dev/null

mkdir -p $BUILD
pushd $BUILD > /dev/null
BUILD=`pwd`
popd > /dev/null

mkdir -p $INSTALL 
pushd $INSTALL > /dev/null
INSTALL=`pwd`
popd > /dev/null

copy_dir_clean $DIST $SRC
check_success "Copying glibc distribution to working directory"

if [[ "$PRODUCT_VERSION" == "v3" ]]; then
	mv ${SRC}/ports ${SRC}/libc/
	check_success "Moving the ports subdirectory to proper location"
fi

cd $BUILD

### Configure glibc. ###
CFLAGS='-g -O2 -fipa-sra' \
CXXFLAGS='-g -O2 -fipa-sra' \
${SRC}/libc/configure --prefix=/usr \
  --host=$BUILD_TARGET \
  --with-pkgversion=${PKG_VERSION} \
  --${FP} \
  --enable-add-ons=nptl,ports

check_success "Configuring glibc"

### Build glibc. ###
CFLAGS='-g -O2 -fipa-sra' \
CXXFLAGS='-g -O2 -fipa-sra' \
make -j
check_success "Building glibc"

### Install glibc. ###
CFLAGS='-g -O2 -fipa-sra' \
CXXFLAGS='-g -O2 -fipa-sra' \
make install_root=${INSTALL} install
check_success "Installing glibc"

cd $TOP
