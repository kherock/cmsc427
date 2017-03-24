After compiling, you can see all the different ways your program can be called by executing your program with no parameters or with the -help flag

./cmsc427
./cmsc427 -help

The following is an examples of how your program could be called from the command line to invoke the crop operation. For more explanation of what the arguments mean, read the source code.

./cmsc427 Checkerboard.jpg CroppedCheckerboard.jpg -crop 20 30 50 100


NOTE: The above commands will work for Linux/Mac terminals and assume the images are in the same directory as the executable. For Windows, the executable file may be placed in a "debug" or "release" folder due to how Visual Studio compiles source code.
