#!/bin/bash

### Global constants ###
BASE_CPU_TYPE="\"7-A\""

### Script internal variables. ###
SEARCH_PATH=.
MODEL=GOLFP

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
}

# Check if the file has the .comment section
# Returns 0 if not stripped and 1 if it is stripped
check_stripped ()
{
  ${CROSS_READELF} -p ".comment" $1 2>/dev/null | grep "String dump of section" > /dev/null
  if [ $? == 1 ]
  then
    echo "[WARNING: No toolchain version - check this file manually] $name"
    continue
  fi
}

# Check the object file architecture
# Returns 1 if the object arch is differ from given
check_arch ()
{
  ${CROSS_READELF} -h $1 | grep "Machine:" | grep "$ARCH" > /dev/null
  if [ $? == 1 ]
  then
    echo "[PROBLEM: ${ARCH} toolchain is not found] $name"
    continue
  fi
}

# Check the compiler tune options
check_cpu_type ()
{
  FILE_CPU_TYPE_LIST=`${CROSS_READELF} -A $1 | grep -e Tag_CPU_name -e Tag_GNU_MIPS_ABI_FP | sed -e "s/^.*Tag_.*:\ //g" | sort -u | grep -v "${CPU_TYPE}"`
  for FILE_CPU_TYPE in $FILE_CPU_TYPE_LIST; do
    echo "${FILE_CPU_TYPE}" | grep "${BASE_CPU_TYPE}" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
      echo "[WARNING: `echo -n ${FILE_CPU_TYPE}` CPU type used] $name"
    else
      echo "[PROBLEM: `echo -n ${FILE_CPU_TYPE}` CPU type used] $name"
    fi
  done
}

# Check the toolchain version
check_toolchain_version ()
{
  FILE_TOOLCHAIN_VERSION_LIST=`${CROSS_READELF} -p ".comment" $1 2>/dev/null | grep GCC: | sed -e "s/^.*GCC:\ //g" | sort -u | grep -v -e "${TOOLCHAIN_VERSION}" -e "${INTERNAL_TOOLCHAIN_VERSION}"`
  if [[ ! -z "${FILE_TOOLCHAIN_VERSION_LIST}" ]]
  then
    echo "[PROBLEM: `echo -n ${FILE_TOOLCHAIN_VERSION_LIST}` toolchain version used] $name"
    continue
  fi
}

# Print the help message
help () {
  echo "Usage: $0 [-p SEARCH_PATH] [-m MODEL]"
  echo "Check if the object files are built using the wrong toolchain version"
  echo
  echo "  -p set the search path (by default the current path will be used)" 
  echo "  -m set the product model name (by default $MODEL will be used)"
  echo "     the following values are supported: FOXP, FOXB, X12, NT13, X13, GOLFP, GOLFS, GOLFV"
  echo "  -h print this message"
  echo
}

### Parse command line options. ###
while getopts "p:m:h" OPTION
do
  case $OPTION in
    p)
      SEARCH_PATH="$OPTARG"
      ;;
    m)
      MODEL="$OPTARG"
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

### Check required toolchain version ###
case $MODEL in
"ECHOP")
  ARCH="ARM"
  CPU_TYPE="7-A"
  BUILD_TARGET=arm-v7a8v2r2-linux-gnueabi
  TOOLCHAIN_VERSION="VDLinux GA-release2 OpenMP 2011-06-30"
  INTERNAL_TOOLCHAIN_VERSION="VDLinux GA-release2 OpenMP 2011-06-30"
  ;;
"FOXB")
  ARCH="ARM"
  CPU_TYPE="Cortex-A8"
  BUILD_TARGET=arm-v7a8v3r1-linux-gnueabi
  TOOLCHAIN_VERSION="VDLinux.GA1.2012-10-03"
  INTERNAL_TOOLCHAIN_VERSION="Linaro GCC 4.6"
  ;;
"X12")
  ARCH="ARM"
  CPU_TYPE="Cortex-A8"
  BUILD_TARGET=arm-v7a8v3r1-linux-gnueabi
  TOOLCHAIN_VERSION="VDLinux.GA1.2012-10-03"
  INTERNAL_TOOLCHAIN_VERSION="Linaro GCC 4.6"
  ;;
"FOXP")
  ARCH="ARM"
  CPU_TYPE="Cortex-A15"
  BUILD_TARGET=arm-v7a15v3r1-linux-gnueabi
  TOOLCHAIN_VERSION="VDLinux.GA1.2012-10-03"
  INTERNAL_TOOLCHAIN_VERSION="Linaro GCC 4.6"
  ;;
"NT13")
  ARCH="MIPS"
  CPU_TYPE="Soft float"
  BUILD_TARGET=mipsel-24kv3r1-linux-gnu
  TOOLCHAIN_VERSION="VDLinux.GA1.2012-10-03"
  INTERNAL_TOOLCHAIN_VERSION="Linaro GCC 4.6"
  ;;
"X13")
  ARCH="MIPS"
  CPU_TYPE="Hard float"
  BUILD_TARGET=mipsel-34kv3r1-linux-gnu
  TOOLCHAIN_VERSION="VDLinux.GA1.2012-10-03"
  INTERNAL_TOOLCHAIN_VERSION="Linaro GCC 4.6"
  ;;
"GOLFP")
  ARCH="ARM"
  CPU_TYPE="Cortex-A15"
  BUILD_TARGET=arm-v7a15v4r3-linux-gnueabi
  TOOLCHAIN_VERSION="VDLinux.v7a15.GA3.2013-10-11"
  INTERNAL_TOOLCHAIN_VERSION="Linaro GCC 4.7"
  ;;
"GOLFS")
  ARCH="ARM"
  CPU_TYPE="Cortex-A8"
  BUILD_TARGET=arm-v7a8v4r3-linux-gnueabi
  TOOLCHAIN_VERSION="VDLinux.v7a8.GA3.2013-10-11"
  INTERNAL_TOOLCHAIN_VERSION="Linaro GCC 4.7"
  ;;
"GOLFV")
  ARCH="ARM"
  CPU_TYPE="Cortex-A8"
  BUILD_TARGET=arm-v7a8v4r3-linux-gnueabi
  TOOLCHAIN_VERSION="VDLinux.v7a8.GA3.2013-10-11"
  INTERNAL_TOOLCHAIN_VERSION="Linaro GCC 4.7"
  ;;
*)
  echo "Error. Unknown product model."
  echo "To get help run $0 -h"
  exit -1
esac

CROSS_READELF=${CROSS_READELF:-${BUILD_TARGET}-readelf}

### Check the cross-readelf. ###
$CROSS_READELF -v >/dev/null
check_success "Checking the readelf utility in \$PATH"

### General information ###
echo "Checking each object-file is built for ${MODEL} target:"
echo "  -target CPU architecture should be ${ARCH}"
echo "  -target CPU type should be ${CPU_TYPE}"
echo "  -toolchain version should be ${TOOLCHAIN_VERSION}"
echo ""

### Generate the file list ###
echo "Generating the file list..."
FILE_LIST=`find -L $SEARCH_PATH -type f \( -name "*.so" -o -name "*.o" -o -name "*.a" \)`

### Check files ###
echo "Checking files..."
for name in $FILE_LIST
do
  check_stripped $name
  check_arch $name
  check_toolchain_version $name
  check_cpu_type $name
  #echo "[OK] $name"
done

