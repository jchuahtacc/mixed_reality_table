#!/bin/sh
for i in "$@"
do
case $i in
    -c=*)
    CAMPARAMS="${i#*=}"
    shift # past argument=value
    ;;
    --dp=*)
    DETECTORPARAMS="${i#*=}"
    shift # past argument=value
    ;;
    -i=*)
    IMAGE="${i#*=}"
    shift # past argument=value
    ;;
    -v=*)
    VIDEO="${i#*=}"
    shift # past argument=value
    ;;
    -d=*)
    DICTIONARY="${i#*=}"
    shift # past argument=value
    ;;
    --adaptiveThreshWin)
    ADAPTIVETHRESHWIN=YES
    shift
    ;;
    *)
            # unknown option
    ;;
esac
done

for $i in {1..5}
do
    echo "$i"
done
