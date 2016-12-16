### Building this package

This project requires OpenCV3, with contrib modules. You will build this project using `cmake`. You will need to know the location of your OpenCV3 installation, including the subdirectory containing the `.cmake` files. For example, the location of this directory when you have installed OpenCV3 using `brew` is `/usr/local/opt/opencv3/share/OpenCV/`. Here are example build instructions:

```
mkdir build
cd build
cmake -D OpenCV_DIR=/usr/local/opt/opencv3/share/ ..
make
```

