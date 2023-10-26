#!/bin/bash
g++ find_flag.c -lcurl -o lab2.exe;
flag=$(./lab2.exe);
echo $flag;
curl https://inp.zoolab.org/binflag/verify?v=$flag;