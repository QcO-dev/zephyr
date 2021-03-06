![Zephyr Logo](https://repository-images.githubusercontent.com/492445571/0fa531e6-78ef-4054-a564-f6673f641ea5)

# Zephyr Programming Language

Zephyr is a statically, [strongly](https://en.wikipedia.org/wiki/Strong_and_weak_typing) typed language which compiles to assembly.
Currently supported is [Linux](https://en.wikipedia.org/wiki/Linux) [x86_64](https://en.wikipedia.org/wiki/X86-64).

## *Zephyr is currently WIP and highly likely to change - use at your own risk!*

## Installation / Bootstrapping

As Zephyr is self-hosted, it must be [bootstrapped](https://en.wikipedia.org/wiki/Bootstrapping_(compilers)) first.

The assembly files in the ./bootstrap directory can be used for this.

Firstly, you must have [yasm](https://yasm.tortall.net/) installed into your `$PATH`.

### Note:
To sync the bootstrap files you may need to run:
```console
$ git lfs pull
```

### With make

```console
$ make bootstrap

$ ./build/zephyr -o ./build/zephyr ./src/main.zpr
$ ./build/zephyr -o ./build/zephyr ./src/main.zpr
$ ./build/zephyr -o ./build/zephyr ./src/main.zpr
$ ...
```

### Without make

```console
$ yasm -felf64 -o ./bootstrap/bootstrap_x86_64_linux.o ./bootstrap/bootstrap_x86_64_linux.yasm
$ ld -o ./bootstrap/bootstrap_x86_64_linux ./bootstrap/bootstrap_x86_64_linux.o

$ ./bootstrap/bootstrap_x86_64_linux -o ./build/zephyr ./src/main.zpr

$ ./build/zephyr -o ./build/zephyr ./src/main.zpr
$ ./build/zephyr -o ./build/zephyr ./src/main.zpr
$ ./build/zephyr -o ./build/zephyr ./src/main.zpr
$ ...
```

## Examples

### Hello, World
```
import "std/io.zpr";

function main(): int {
	putsln("Hello, World!");
	return 0;
}
```

### File Write
```
import "std/io.zpr";

function main(): int {
	var file = fopen("./myfile.txt", 'w');
	file.putsln("Hello, Zephyr FileIO!");
	file.close();
	return 0;
}
```

### Struct Methods
```
import "std/io.zpr";

struct Person {
	name: i8*;
}

function Person.constructor(name: i8*) {
	this.name = name;
}

function Person.say_hi() {
	puts("Hi, I'm "); puts(this.name); putsln("!"); // f"Hi, I'm {name}!\n"
}

function main(): int {
	var alice: Person("Alice");

	alice.say_hi();

	return 0;
}
```

### More Examples

See ./examples for more examples of the language.

## Testing

Running the bash scripts in ./tests can be used to test the language.

Running ./tests/all.sh will run all the tests and give a count of the
successful, total, and skipped tests.

### Note:
Due to git, you may need to change the line endings of each test file to LF instead of CRLF.
