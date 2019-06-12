# simple-c-compiler

## Overview

Simple C is a subset of the C programming language. That is, all Simple C programs are valid C programs. Simple C uses a recursive descent parser that moves from left-to-right, and performs the leftmost derivation. In other words, the Simple C parser is an LL(k) parser.

The Simple C compiler generates valid assembly code exclusively for 64-bit Intel machines running the Linux operating system.

## Phase 1

Lexical analyzer

## Phase 2

Recursive descent parser for entire Simple C language

## Phase 3

Scope and symbol table with rudimentary type checking

## Phase 4

Type checking for entire Simple C language

## Phase 5

Storage allocation for functions and rudimentary code generation

## Phase 6

Code generation for entire Simple C language
