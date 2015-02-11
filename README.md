# vis2ascii

vis2ascii is a command line tool that converts a Commodore 64 assembly source file in

* [VisAss](http://csdb.dk/release/?id=6166) format or
* [F8 AssBlaster](http://csdb.dk/release/?id=126584) format

into ASCII, so it can be used with modern cross-development tools.

The format of the input file is auto-detected.

## Usage
    vis2ascii "test		.src"
creates test.asm.

    vis2ascii "test       .src" test.txt
creates test.txt.

    vis2ascii "text       .src" -
prints text to stdout.

## Notes

* vis2ascii automatically strips the spaces between the name and the extension if no target name is given.

* vis2ascii cannot extract files from .D64 images. Use a tool like c1541 to extract the files:

    `echo extract | c1541 disk.d64`
    `for i in *.src; do vis2ascii "$i"; done`

* vis2ascii only converts the VisAss/F8 AssBlaster file format into ASCII, it does not convert the assembly conventions into another format. That is, pseudo-opcodes and macros stay in the original format, but converting this manually should be easy.

* If you are only converting a single file, you can also use the "print" functionality in the respective orginal editors inside an emulator to export the file to ASCII, but this tool is much more handy when converting all your old sources.

## TODO

Adding regular AssBlaster support should be easy; the AssBlaster format seems to be the F8 AssBlaster format with the VisAss opcode table. The two formats can be distinguished by the gamut of used menmonics.

## License

[2-clause BSD](http://opensource.org/licenses/BSD-2-Clause)

## Author

Michael Steil, [pagetable.com](http://www.pagetable.com/)
