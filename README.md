# Decoding PNG to PNM

This program decodes a PNG image into a PNG image. 

------------

Gray, color and palette images (Color Type 0, 2 and 3), 8 bits per channel are supported.
Supported libraries: zlib, zlib deflate and isa-l.

------------

To run this when needed: 
*(Pre-install the library you need)*
1. Clone the repository
2. Next, you need to select a library and compile the file depending on the library (select one of the commands below and enter into the terminal):
> - *zlib:* **gcc -o main main.c -L[path to zlib] -lz**
> - *libdeflate:* **gcc -o main main.c -L[путь до libdeflate] -ldeflate -DLIBDEFLATE**
> - *isa-l:* **gcc -o main main.c -L=[путь до isa-l] -DISAL -lisal**

3. To run the code, you need to specify a PNG file for input and the name for the PNG file that will be created or overwritten
> - **./main input_file output_file**

