#!/bin/bash
for i in "$@"
do
case $i in
    --bp=*)
    BLOBPARAMS="${i#*=}"
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
    *)
            # unknown option
    ;;
esac
done

for minCircularity in $(seq 0.1 0.1 1); do
    for maxCircularity in $(seq $minCircularity 0.1 1); do
        for minConvexity in $(seq 0.1 0.1 1); do
            echo "${minConvexity}"
            for maxConvexity in $(seq $minConvexity 0.1 1); do
                for minInertiaRatio in $(seq 0.1 0.1 1); do
                    for maxInertiaRatio in $(seq $minInertiaRatio 0.1 1); do
                        #echo "===="
                        detected=$(./detect_blobs -i=${IMAGE} --op=filterByCircularity=true,minCircularity=${minCircularity},maxCircularity=${maxCircularity},filterByConvexity=true,minConvexity=${minConvexity},maxConvexity=${maxConvexity},filterByInertia=true,minInertiaRatio=${minInertiaRatio},maxInertiaRatio=${maxInertiaRatio} | grep "Blobs detected: 0")
                        if [ -z ${detected+x} ]; then
                            echo "Found blobs"
                            echo "./detect_blobs -i=${IMAGE} --op=filterByCircularity=true,minCircularity=${minCircularity},maxCircularity=${maxCircularity},filterByConvexity=true,minConvexity=${minConvexity},maxConvexity=${maxConvexity},filterByInertia=true,minInertiaRatio=${minInertiaRatio},maxInertiaRatio=${maxInertiaRatio}"
                            ./detect_blobs -i=${IMAGE} --op=filterByCircularity=true,minCircularity=${minCircularity},maxCircularity=${maxCircularity},filterByConvexity=true,minConvexity=${minConvexity},maxConvexity=${maxConvexity},filterByInertia=true,minInertiaRatio=${minInertiaRatio},maxInertiaRatio=${maxInertiaRatio}
                        fi
                    done
                done
            done
        done
    done
done
