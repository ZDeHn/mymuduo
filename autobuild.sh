set -e

if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

cd ..

if [ ! -d /usr/include/muduozdh ]; then 
    mkdir /usr/include/muduozdh
fi

cd `pwd`/src/include

for header in `ls *.h`
do
    cp $header /usr/include/muduozdh
done

cd ..
cd ..

sudo cp `pwd`/lib/libmuduozdh.a /usr/lib

if [ ! -d `pwd`/HttpServer/build ]; then
    mkdir `pwd`/HttpServer/build
fi

rm -rf `pwd`/HttpServer/build/*

cd `pwd`/HttpServer/build &&
    cmake .. &&
    make

cd ..
cd ..

if [ ! -d `pwd`/SkipListServer/build ]; then
    mkdir `pwd`/SkipListServer/build
fi

rm -rf `pwd`/SkipListServer/build/*

cd `pwd`/SkipListServer/build &&
    cmake .. &&
    make

cd ..
cd ..

ldconfig