#!/bin/bash

set -ev

lein run build-jni-java
lein run build-jni
