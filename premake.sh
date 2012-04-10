#!/bin/sh

ALBION_OS="unknown"
ALBION_PLATFORM="x32"
ALBION_MAKE="make"
ALBION_MAKE_PLATFORM="32"
ALBION_ARGS=""
ALBION_CPU_COUNT=1
ALBION_USE_CLANG=0

if [[ $# > 0 && $1 == "gcc" ]]; then
	ALBION_ARGS="--gcc"
else
	ALBION_ARGS="--clang"
	ALBION_USE_CLANG=1
fi

case $( uname | tr [:upper:] [:lower:] ) in
	"darwin")
		ALBION_OS="macosx"
		ALBION_CPU_COUNT=$(sysctl -a | grep 'machdep.cpu.thread_count' | sed -E 's/.*(: )([:digit:]*)/\2/g')
		;;
	"linux")
		ALBION_OS="linux"
		ALBION_CPU_COUNT=$(cat /proc/cpuinfo | grep -m 1 'cpu cores' | sed -E 's/.*(: )([:digit:]*)/\2/g')
		;;
	[a-z0-9]*"BSD")
		ALBION_OS="bsd"
		ALBION_MAKE="gmake"
		# TODO: get cpu/thread count on *bsd
		;;
	"cygwin"*)
		ALBION_OS="windows"
		ALBION_ARGS+=" --env cygwin"
		ALBION_CPU_COUNT=$(env | grep 'NUMBER_OF_PROCESSORS' | sed -E 's/.*=([:digit:]*)/\1/g')
		;;
	"mingw"*)
		ALBION_OS="windows"
		ALBION_ARGS+=" --env mingw"
		ALBION_CPU_COUNT=$(env | grep 'NUMBER_OF_PROCESSORS' | sed -E 's/.*=([:digit:]*)/\1/g')
		;;
	*)
		echo "unknown operating system - exiting"
		exit
		;;
esac


case $( uname -m ) in
	"i386"|"i486"|"i586"|"i686")
		ALBION_PLATFORM="x32"
		ALBION_MAKE_PLATFORM="32"
		ALBION_ARGS+=" --platform x32"
		;;
	"x86_64"|"amd64")
		ALBION_PLATFORM="x64"
		ALBION_MAKE_PLATFORM="64"
		ALBION_ARGS+=" --platform x64"
		;;
	*)
		echo "unknown architecture - using "${ALBION_PLATFORM}
		exit;;
esac


echo "using: premake4 --cc=gcc --os="${ALBION_OS}" gmake "${ALBION_ARGS}

premake4 --cc=gcc --os=${ALBION_OS} gmake ${ALBION_ARGS}
sed -i -e 's/\${MAKE}/\${MAKE} -j '${ALBION_CPU_COUNT}'/' Makefile

if [[ $ALBION_USE_CLANG == 1 ]]; then
	sed -i '1i export CC=clang' Makefile
	sed -i '1i export CXX=clang++' Makefile
fi

echo ""
echo "###################################################"
echo "# NOTE: use '"${ALBION_MAKE}" config=release"${ALBION_MAKE_PLATFORM}"' to build albion"
echo "###################################################"
echo ""
