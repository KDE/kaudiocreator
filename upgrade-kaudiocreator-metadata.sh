#! /usr/bin/env bash

while read; do
    if [ "${REPLY#\[}" != "$REPLY" ] ; then # group name
	GROUP="${REPLY:1:${#REPLY}-2}"
	continue;
    fi
    # normal key=value pair:
    KEY="${REPLY%%=*}"
    VALUE="${REPLY#*=}"

     case "$GROUP/$KEY" in
	encoderconfig/fileFormat)
      VALUE=`echo $VALUE | sed -e s/%album/%{albumtitle}/g`;
      VALUE=`echo $VALUE | sed -e s/%artist/%{albumartist}/g`;
      VALUE=`echo $VALUE | sed -e s/%song/%{title}/g`;
      VALUE=`echo $VALUE | sed -e s/%track/%{number}/g`;
      VALUE=`echo $VALUE | sed -e s/%genre/%{genre}/g`;
      VALUE=`echo $VALUE | sed -e s/%extension/%{extension}/g`;
      VALUE=`echo $VALUE | sed -e s/%year/%{year}/g`;
	    echo "[encoderconfig]";
	    echo "fileFormat=$VALUE"
	    echo "# DELETE [encoderconfig]fileFormat"
			;;

	Encoder_0/commandLine)
      VALUE=`echo $VALUE | sed -e s/%album/%{albumtitle}/g`;
      VALUE=`echo $VALUE | sed -e s/%artist/%{albumartist}/g`;
      VALUE=`echo $VALUE | sed -e s/%song/%{title}/g`;
      VALUE=`echo $VALUE | sed -e s/%track/%{number}/g`;
      VALUE=`echo $VALUE | sed -e s/%genre/%{genre}/g`;
      VALUE=`echo $VALUE | sed -e s/%extension/%{extension}/g`;
      VALUE=`echo $VALUE | sed -e s/%year/%{year}/g`;
	    echo "[Encoder_0]";
	    echo "commandLine=$VALUE"
	    echo "# DELETE [Encoder_0]commandLine"
			;;

	Encoder_1/commandLine)
      VALUE=`echo $VALUE | sed -e s/%album/%{albumtitle}/g`;
      VALUE=`echo $VALUE | sed -e s/%artist/%{albumartist}/g`;
      VALUE=`echo $VALUE | sed -e s/%song/%{title}/g`;
      VALUE=`echo $VALUE | sed -e s/%track/%{number}/g`;
      VALUE=`echo $VALUE | sed -e s/%genre/%{genre}/g`;
      VALUE=`echo $VALUE | sed -e s/%extension/%{extension}/g`;
      VALUE=`echo $VALUE | sed -e s/%year/%{year}/g`;
	    echo "[Encoder_1]";
	    echo "commandLine=$VALUE"
	    echo "# DELETE [Encoder_1]commandLine"
			;;

	Encoder_2/commandLine)
      VALUE=`echo $VALUE | sed -e s/%album/%{albumtitle}/g`;
      VALUE=`echo $VALUE | sed -e s/%artist/%{albumartist}/g`;
      VALUE=`echo $VALUE | sed -e s/%song/%{title}/g`;
      VALUE=`echo $VALUE | sed -e s/%track/%{number}/g`;
      VALUE=`echo $VALUE | sed -e s/%genre/%{genre}/g`;
      VALUE=`echo $VALUE | sed -e s/%extension/%{extension}/g`;
      VALUE=`echo $VALUE | sed -e s/%year/%{year}/g`;
	    echo "[Encoder_2]";
	    echo "commandLine=$VALUE"
	    echo "# DELETE [Encoder_2]commandLine"
			;;			
	Encoder_3/commandLine)
      VALUE=`echo $VALUE | sed -e s/%album/%{albumtitle}/g`;
      VALUE=`echo $VALUE | sed -e s/%artist/%{albumartist}/g`;
      VALUE=`echo $VALUE | sed -e s/%song/%{title}/g`;
      VALUE=`echo $VALUE | sed -e s/%track/%{number}/g`;
      VALUE=`echo $VALUE | sed -e s/%genre/%{genre}/g`;
      VALUE=`echo $VALUE | sed -e s/%extension/%{extension}/g`;
      VALUE=`echo $VALUE | sed -e s/%year/%{year}/g`;
	    echo "[Encoder_3]";
	    echo "commandLine=$VALUE"
	    echo "# DELETE [Encoder_3]commandLine"
			;;			
    esac
done
