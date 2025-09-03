

Translation unit:

One source file, after including all its headers and processing all macros, that the compiler compiles at once.

In simple words:

- You write a .c or .cpp file.

- It may include other files using #include.

- The compiler takes your file + all included code and turns it into a single "big file" internally.

- That big file is the translation unit.

Each translation unit is compiled separately into an object file (.o or .obj), and then the linker combines all those object files into your final program.
