#!/bin/bash

rm test/rtest.out test/test.out

for i in {01..16}
do
    make rtest$i >> test/rtest.out
    make test$i >> test/test.out
done