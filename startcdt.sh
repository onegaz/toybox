#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#PATH=/usr/lib/jvm/java-9-openjdk-amd64/bin:$PATH

eclipse -data $DIR 2>/dev/null &
