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
    sudo mkdir /usr/include/muduozdh
fi

cd `pwd`/src/include

for header in `ls *.h`
do
    sudo cp $header /usr/include/muduozdh
done

cd ..
cd ..

cd `pwd`/lib


sudo cp libmuduozdh.a /usr/local/lib