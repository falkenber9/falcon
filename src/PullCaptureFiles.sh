#!/bin/bash


SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

DST_PATH=.

REMOTES=(
	cni-lte5.local
	cni-lte6.local
	cni-lte9.local
)

REMOTE_SRC_PATH=(
	/home/cni-lte/falcon/build/src
	/home/cni-lte/falcon/build/src
	/home/cni/falcon/build/src
)

USERS=(
	cni-lte
	cni-lte
	cni
)


SYNCTOOL=rsync
SYNCTOOL_PARAMS="-aHAXxv --numeric-ids --remove-source-files --progress"
#SYNCTOOL_PARAMS="-aHAXxv --numeric-ids --progress"

i=0
for REMOTE in ${REMOTES[@]}
do
	USER=${USERS[$i]}
	SRC_PATH=${REMOTE_SRC_PATH[$i]}
	echo "Pulling from remote '$REMOTE'"
	echo "User: $USER"
	echo "SRC_PATH: $SRC_PATH"
	echo "DST_PATH: $DST_PATH"
	mkdir -p $DST_PATH/$REMOTE
	echo "$SYNCTOOL $SYNCTOOL_PARAMS $USER@$REMOTE:$SRC_PATH/*.{csv,bin,log} $DST_PATH/$REMOTE"
	$SYNCTOOL $SYNCTOOL_PARAMS $USER@$REMOTE:$SRC_PATH/*.{csv,bin,log} $DST_PATH/$REMOTE
	
	i=$((i+1))
done;

#rsync -aHAXxv --numeric-ids --delete --progress -e "ssh -T -c arcfour -o Compression=no -x" user@<source>:<source_dir> <dest_dir>
