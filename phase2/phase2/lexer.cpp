/*
 * File:	lexer.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the lexical analyzer for Simple C.
 */

# include <iterator>
# include <set>
# include <cstdio>
# include <cerrno>
# include <cctype>
# include <cstdlib>
# include <iostream>

# include "tokens.h"
# include "lexer.h"

using namespace std;

int numerrors, lineno = 1;


/* Later, we will associate token values with each keyword */

static set<string> keywords = {
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "int",
    "long",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
};


/*
 * Function:	report
 *
 * Description:	Report an error to the standard error prefixed with the
 *		line number.  We'll be using this a lot later with an
 *		optional string argument, but C++'s stupid streams don't do
 *		positional arguments, so we actually resort to snprintf.
 *		You just can't beat C for doing things down and dirty.
 */

void report(const string &str, const string &arg)
{
    char buf[1000];

    snprintf(buf, sizeof(buf), str.c_str(), arg.c_str());
    cerr << "line " << lineno << ": " << buf << endl;
    numerrors ++;
}


/*
 * Function:	lexan
 *
 * Description:	Read and tokenize the standard input stream.  The lexeme is
 *		stored in a buffer.
 */

int lexan(string &lexbuf)
{
  static int c = cin.get();


  /* The invariant here is that the next character has already been read
      and is ready to be classified.  In this way, we eliminate having to
      push back characters onto the stream, merely to read them again. */

  while (!cin.eof()) {
	  lexbuf.clear();


    /* Ignore white space */

    while (isspace(c)) {
      if (c == '\n')
        lineno ++;

      c = cin.get();
    }


    /* Check for an identifier or a keyword */

    if (isalpha(c) || c == '_') {
      do {
        lexbuf += c;
        c = cin.get();
      } while (isalnum(c) || c == '_');

      if (keywords.count(lexbuf) > 0) {
        if (lexbuf.compare("auto") == 0)
          return AUTO;

        if (lexbuf.compare("break") == 0)
          return BREAK;

        if (lexbuf.compare("case") == 0)
          return CASE;

        if (lexbuf.compare("char") == 0)
          return CHAR;

        if (lexbuf.compare("const") == 0)
          return CONST;

        if (lexbuf.compare("continue") == 0)
          return CONTINUE;

        if (lexbuf.compare("default") == 0)
          return DEFAULT;

        if (lexbuf.compare("do") == 0)
          return DO;

        if (lexbuf.compare("double") == 0)
          return DOUBLE;

        if (lexbuf.compare("else") == 0)
          return ELSE;

        if (lexbuf.compare("enum") == 0)
          return ENUM;

        if (lexbuf.compare("extern") == 0)
          return EXTERN;

        if (lexbuf.compare("float") == 0)
          return FLOAT;

        if (lexbuf.compare("for") == 0)
          return FOR;

        if (lexbuf.compare("goto") == 0)
          return GOTO;

        if (lexbuf.compare("if") == 0)
          return IF;

        if (lexbuf.compare("int") == 0)
          return INT;

        if (lexbuf.compare("long") == 0)
          return LONG;

        if (lexbuf.compare("register") == 0)
          return REGISTER;

        if (lexbuf.compare("return") == 0)
          return RETURN;

        if (lexbuf.compare("short") == 0)
          return SHORT;

        if (lexbuf.compare("signed") == 0)
          return SIGNED;

        if (lexbuf.compare("sizeof") == 0)
          return SIZEOF;

        if (lexbuf.compare("static") == 0)
          return STATIC;

        if (lexbuf.compare("struct") == 0)
          return STRUCT;

        if (lexbuf.compare("switch") == 0)
          return SWITCH;

        if (lexbuf.compare("typedef") == 0)
          return TYPEDEF;

        if (lexbuf.compare("union") == 0)
          return UNION;

        if (lexbuf.compare("unsigned") == 0)
          return UNSIGNED;

        if (lexbuf.compare("void") == 0)
          return VOID;

        if (lexbuf.compare("volatile") == 0)
          return VOLATILE;

        if (lexbuf.compare("while") == 0)
          return WHILE;
      }

      return ID;


    /* Check for a number */

    } else if (isdigit(c)) {
        do {
      lexbuf += c;
      c = cin.get();
        } while (isdigit(c));

        errno = 0;
        strtol(lexbuf.c_str(), NULL, 0);

        if (errno != 0)
      report("integer constant too large");

        return NUM;


    /* There must be an easier way to do this.  It might seem stupid at
      this point to recognize each token separately, but eventually
      we'll be returning separate token values to the parser, so we
      might as well do it now. */

    } else {
        lexbuf += c;

        switch(c) {


        /* Check for '||' */

          case '|':
        c = cin.get();

        if (c == '|') {
            lexbuf += c;
            c = cin.get();
            return OR;
        }
        return ERROR;


          /* Check for '=' and '==' */

          case '=':
        c = cin.get();

        if (c == '=') {
            lexbuf += c;
            c = cin.get();
            return EQL;
        }

        return '=';


          /* Check for '&' and '&&' */

          case '&':
        c = cin.get();

        if (c == '&') {
            lexbuf += c;
            c = cin.get();
            return AND;
        }

        return '&';


          /* Check for '!' and '!=' */

          case '!':
        c = cin.get();

        if (c == '=') {
            lexbuf += c;
            c = cin.get();
            return NEQ;
        }

        return '!';


          /* Check for '<' and '<=' */

          case '<':
        c = cin.get();

        if (c == '=') {
            lexbuf += c;
            c = cin.get();
            return LEQ;
        }

        return '<';


          /* Check for '>' and '>=' */

          case '>':
        c = cin.get();

        if (c == '=') {
            lexbuf += c;
            c = cin.get();
            return GEQ;
        }

        return '>';


          /* Check for '-', '--', and '->' */

          case '-':
        c = cin.get();

        if (c == '-') {
            lexbuf += c;
            c = cin.get();
            return DEC;
        } else if (c == '>') {
            lexbuf += c;
            c = cin.get();
            return ARROW;
        }

        return '-';


          /* Check for '+' and '++' */

          case '+':
        c = cin.get();

        if (c == '+') {
            lexbuf += c;
            c = cin.get();
            return INC;
        }

        return '+';


          /* Check for simple, single character tokens */

          case '*': case '%': case ':': case ';':
          case '(': case ')': case '[': case ']':
          case '{': case '}': case '.': case ',':
        c = cin.get();
        return lexbuf[0];


          /* Check for '/' or a comment */

          case '/':
        c = cin.get();

        if (c == '*') {
            do {
          while (c != '*' && !cin.eof()) {
              if (c == '\n')
            lineno ++;

              c = cin.get();
          }

          c = cin.get();
            } while (c != '/' && !cin.eof());

            c = cin.get();
            break;

        } else if (c == '/') {
            c = cin.get();

            while (c != '\n' && !cin.eof())
          c = cin.get();

            break;

        } else
            return '/';


          /* Check for a string literal */

          case '"':
        c = cin.get();

        while (c != '"' && c != '\n' && !cin.eof()) {
            lexbuf += c;
            c = cin.get();
        }

        if (c == '\n' || cin.eof())
            report("premature end of string literal");

        lexbuf +=c ;
        c = cin.get();
        return STRING;


          /* Handle EOF here as well */

          case EOF:
        return DONE;


          /* Everything else is illegal */

          default:
        c = cin.get();
        break;
        }
    }
  }

  return ERROR;
}


/*
 * Function:	main
 *
 * Description:	Read and tokenize and standard input stream.
 */

/*int main()
{
    string lexbuf;

    while (lexan(lexbuf))
	continue;

    return 0;
}*/
