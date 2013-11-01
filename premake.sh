#!/bin/sh

ALBION_OS="unknown"
ALBION_PLATFORM="x32"
ALBION_MAKE="make"
ALBION_MAKE_PLATFORM="32"
ALBION_ARGS=""
ALBION_CPU_COUNT=1
ALBION_USE_CLANG=1

for arg in "$@"; do
	case $arg in
		"gcc")
			ALBION_ARGS+=" --gcc"
			ALBION_USE_CLANG=0
			;;
		"cuda")
			ALBION_USE_CLANG+=" --cuda"
			;;
		*)
			;;
	esac
done

if [[ $ALBION_USE_CLANG == 1 ]]; then
	ALBION_ARGS+=" --clang"
fi

case $( uname | tr [:upper:] [:lower:] ) in
	"darwin")
		ALBION_OS="macosx"
		ALBION_CPU_COUNT=$(sysctl -n hw.ncpu)
		;;
	"linux")
		ALBION_OS="linux"
		# note that this includes hyper-threading and multi-socket systems
		ALBION_CPU_COUNT=$(cat /proc/cpuinfo | grep "processor" | wc -l)
		;;
	[a-z0-9]*"BSD")
		ALBION_OS="bsd"
		ALBION_MAKE="gmake"
		ALBION_CPU_COUNT=$(sysctl -n hw.ncpu)
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


ALBION_PLATFORM_TEST_STRING=""
if [ $ALBION_OS != "windows" ]; then
	ALBION_PLATFORM_TEST_STRING=$( uname -m )
else
	ALBION_PLATFORM_TEST_STRING=$( gcc -dumpmachine | sed "s/-.*//" )
fi

case $ALBION_PLATFORM_TEST_STRING in
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

if [ $ALBION_USE_CLANG == 1 ]; then
	# this seems to be the most portable way of inserting chars in front of a file
	# note that "sed -i "1i ..." file" is not portable!
	mv Makefile Makefile.tmp
	echo "export CC=clang" > Makefile
	echo "export CXX=clang++" >> Makefile
	cat Makefile.tmp >> Makefile
	rm Makefile.tmp
fi

echo ""
echo "###################################################"
echo "# NOTE: use '"${ALBION_MAKE}" config=release"${ALBION_MAKE_PLATFORM}"' to build albion"
echo "###################################################"
echo ""
