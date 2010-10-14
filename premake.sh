#!/bin/sh

ALBION_OS="unknown"
ALBION_PLATFORM="x32"
ALBION_MAKE="make"
ALBION_MAKE_PLATFORM="32"

case $( uname ) in
	"Darwin")
		ALBION_OS="macosx"
		;;
	"Linux")
		ALBION_OS="linux"
		;;
	[a-zA-Z0-9]*"BSD")
		ALBION_OS="bsd"
		ALBION_MAKE="gmake"
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
		;;
	"x86_64"|"amd64")
		ALBION_PLATFORM="x64"
		ALBION_MAKE_PLATFORM="64"
		;;
	*)
		echo "unknown architecture - using "${ALBION_PLATFORM}
		exit;;
esac


echo "using: premake4 --cc=gcc --os="${ALBION_OS}" gmake"

premake4 --cc=gcc --os=${ALBION_OS} gmake

chmod +x build_version.sh

echo ""
echo "###################################################"
echo "# NOTE: use '"${ALBION_MAKE}" config=release"${ALBION_MAKE_PLATFORM}"' to build albion"
echo "###################################################"
echo ""
