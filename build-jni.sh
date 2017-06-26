#!/bin/bash

set -ev

rm java/think/hdf5/*.java

lein run build-jni-java
lein run build-jni
