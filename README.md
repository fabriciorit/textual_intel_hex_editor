# Textual Intel HEX Editor Tool
This code aims to:
 * Edit existing Intel HEX files without changing their regions and functional layout (formatting may be changed).
 * Overwrite data on specific addresses.

Possible uses:
 * Integrate the C++ class TIHex from files TIHex.h and TIHex.cpp in other project.
 * Command line with command TIHex from main.cpp implementation. See building and running section.

Supports common record types as seen in https://en.wikipedia.org/wiki/Intel_HEX

## Build and Running
Make sure you installed CMake version at least 3.0 and a C++ compiler supporting C++11 or greater.

On linux, just run the commands on top directory to build the code:
```sh
mkdir build
cd build
cmake ../
make
```

Examples on Linux terminal:
```sh
tihex your.file.hex -o > output.hex # send stdout to a file. Just copies the file your.file.hex to output.hex

tihex your.file.hex -o | less # pipeline use

echo ":02012300DA7A9A" | build/tihex -i -o # stdin processing using pipeline

echo ":02012300DA7A9A" | build/tihex -i -o -a 0123 -d aa,bb # overwrite data on address 0123 automatically updating checksum.
# Output:
# :02012300AABB75
```

Help command output:
```
Textual Intel Hex Editor
Original author: Fabricio Ribeiro Toloczko (fabriciotoloczko@gmail.com)
file processing:        tihex [options] [input filename]
stdin:                  tihex [options] -i 
--help or -h: this help message.
--stdin or -i: process stdin.
--stdout or -o: show final data on stdout.
--address or -a: set address to overwrite, hexadecimal 0 to FFFFFFFFFFFFFFFF. E.g. "-a EAF00F1".
--data or -d: define data, hex values comma separated. E.g. "-d 0,0,1a,95,AB".
--version or -v: show version.
```

## How it works
Current algorithm:
 1. All data, from a file or stdin, is split in lines.
 2. Each line is parsed into a entry according to Intel HEX format (learn more at https://en.wikipedia.org/wiki/Intel_HEX):
    
    [Start code] [Byte count] [Address] [Record type] [Data] [Checksum]

 3. Each entry is appended to the entry list.
 4. If the entry is a data record (record type 0x00), a reference to it is inserted into the entry map. This map is accessed by a 64 bit address.
 5. Any overwriting is done on entry map according it's address. Because the map is referenced on list entries, all changes are done there too. The checksum is updated automatically (or not, if desired, in C++ class) for each data overwrite.
 6. STL iterators are available to run through the entry list. On command-line tool, the data may be shown to stdout if it's switch is on.


## Planned features
In the future, some features may come:
 * Inserting addresses.
 * Vector overwrite to speed up operations. Today, all overwrites are done byte by byte.
 * Refactoring functions to remove code sections or insert new ones, managing 02,03 and other record types entries.

## Pay me a coffee
If this work is useful to you and you want to thank me, donate on:
 * PIX, for brazilian people, the key is: 33b8f53f-f352-4693-9396-7fdc12cb8ba4
 * Paypal: https://www.paypal.com/donate/?business=4DQWHREP7WNLE&no_recurring=1&item_name=Textual+Intel+HEX+Editor%0Ahttps%3A%2F%2Fgithub.com%2Ffabriciorit%2Ftextual_intel_hex_editor&currency_code=USD
 * Bitcoin: 3LudYaDadgsSBxJfsR8unGrNRxgPa7H6Es
 * Ethereum: 0x73b324300db3de90046cf7039fd3918e0fe37bc6
 * Freewallet, user ID: b3273937

## License
ZLib license.

In short, please don't steal the code and claim it's yours, altough you can use it freely, including commercially. Read the LICENSE.