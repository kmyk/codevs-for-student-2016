#!/bin/bash
cd $(dirname $0)
[ ! -e ./a.out ] && make 1>&2
exec ./a.out
