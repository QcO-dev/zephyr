#!/usr/bin/env bash

. tests/asserts.sh

echo -n "Ternary Expression: "
assert_exit_code 2 "function main(): int { return 0 ? 10 : 2; }"
assert_exit_code 10 "function main(): int { return 1 ? 10 : 2; }"
assert_compilation_error "function x() {} function main: int { return x() ? 1 : 2; }"
echo " Done"

echo -n "If Statements: "
assert_exit_code 5 "
function main(): int {
	var a: int = 0;

	if(1) {
		a = 5;
	}

	return a;
}
"

assert_exit_code 12 "
function main(): int {
	var a: int = 0;

	if(0) {
		a = 5;
	}
	else {
		a = 12;
	}

	return a;
}
"

assert_exit_code 5 "
function main(): int {
	var a: int = 0;

	if(1) {
		a = 5;
	}
	else {
		a = 12;
	}

	return a;
}
"

assert_compilation_error "
function x() {}
function main(): int {
	if(x()) {}

	return 0;
}
"

echo " Done"

echo -n "While Statements: "
assert_stdout "0
1
2
3
4
5
6
7
8
9" "
function main(): int {
	var i: int = 0;

	while(i < 10) {
		printu(i);
		i = i + 1;
	}

	return i - 10;
}
"

assert_compilation_error "function x() {} function main():int { while(x()) {} }"

assert_exit_code 5 "
function main(): int {
	var a: int = 5;

	while(0) a = 12;

	return a;
}
"

echo " Done"