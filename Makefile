CC=gcc
CXX=g++-posix

ifneq (,$(shell which amd64-mingw32msvc-gcc))
	ifeq (${CC}, gcc)
		MING_BASE:=amd64-mingw32msvc-
	else
		MING_BASE:=
	endif
	MINGW_HOST="amd64-mingw32msvc"
else
ifneq (,$(shell which i686-pc-mingw32-gcc))
	ifeq (${CC}, gcc)
		MING_BASE:=i686-pc-mingw32-
	else
		MING_BASE:=
	endif
	MINGW_HOST="i686-pc-mingw32"
else
ifneq (,$(shell which i686-w64-mingw32-gcc))
	ifeq (${CC}, gcc)
		MING_BASE:=i686-w64-mingw32-
	else
		MING_BASE:=
	endif
	MINGW_HOST="i686-w64-mingw32"
else
$(error No windows cross-compiler found!) #MING_BASE:=unknown-
endif
endif
endif

WAZUH_CXX=${MING_BASE}${CXX}

all:
	${WAZUH_CXX} serialNumber.cpp -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual -Wunused -Wcast-align -Wformat=2 -std=c++14 -static-libstdc++ -O3 -s -o SerialNumber.exe
