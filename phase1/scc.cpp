/**
 * Author:  Conner Davis
 * Course:  COEN 175L (Compilers)
 * Section: Tues 2:15 PM
 * Due:     7 Apr 2019 at 11:59 PM
 */

#include <cctype>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

using std::cin;
using std::cout;
using std::set;
using std::size_t;
using std::string;

/* what type of symbol we are currently scanning */
enum State {

  /* indeterminate state */
  undefined,

  /* ignoring everything inside of a comment */
  comment,

  /* reading an identifier, which may be a keyword */
  identifier,

  /* reading a normal or long integer */
  number,

  /* reading a string literal "" */
  text

};

const string COMMENT_CLOSE = "*/";
const string COMMENT_OPEN = "/*";
const string COMMENT_SINGLE = "//";

const set<string> keywords {
  "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", 
  "float", "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", 
  "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

const set<string> operators {
  "=", "|", "||", "&&", "==", "!=", "<", ">", "<=", ">=", "+", "-", "*", "/", "%", "&", "!", "++", "--", ".", 
  "->", "(", ")", "[", "]", "{", "}", ";", ":", ","
};

bool scan_comments( char symbol, char next_symbol );
bool scan_strings( char symbol, char next_symbol );
bool scan_numbers( char symbol, char next_symbol );
bool scan_identifiers( char symbol, char next_symbol );
bool scan_keywords( char symbol, char next_symbol );
bool scan_operators( char symbol, char next_symbol );

void reset( string msg = "" );
void store( char in );
string stored( );
void skip_next( );
void skip_until( char c );

string cache = "";
State state = undefined;

int main( void ) {
  char c;

  while (cin.get( c )) {
    // always store next char in case we need it (likely)
    const char peek = cin.peek( );

    if (scan_comments( c, peek )) continue;
    if (scan_strings( c, peek )) continue;
    if (scan_numbers( c, peek )) continue;
    if (scan_identifiers( c, peek )) continue;
    if (scan_operators( c, peek )) continue;
  }

  return EXIT_SUCCESS;
}

bool scan_comments( char symbol, char symbol_next ) {
  switch (state) {
    // (on close */): skip multi-line comments
    case comment:
      if (symbol == COMMENT_CLOSE[0] && symbol_next == COMMENT_CLOSE[1]) {
        reset( );
        skip_next( );
      }

      return true;

    default:
      /* (on open): skip multi-line comments */
      if (symbol == COMMENT_OPEN[0] && symbol_next == COMMENT_OPEN[1]) 
        state = comment;
      /* (on //): skip one-line comments */
      else if (symbol == COMMENT_SINGLE[0] && symbol_next == COMMENT_SINGLE[1]) 
        skip_until( '\n' );
      else return false;
  }

  return true;
}

bool scan_strings( char symbol, char next_symbol ) {
  switch (state) {
    case text:
      // reached the end of the string
      if (symbol == '"') reset( "string:\"" + stored( ) + "\"" );
      // otherwise continue reading string
      else store( symbol );

      return true;

    default:
      // string has been discovered
      if (symbol == '"') {
        state = text;

        return true;
      }

      return false;
  }
}

bool scan_numbers( char symbol, char next_symbol ) {
  switch (state) {
    /* continuing to read a number */
    case number:
      if (isdigit( next_symbol )) return true;
      /* reached the end of the number */
      else {
        store( symbol );
        
        if ((next_symbol == 'L' || next_symbol == 'l')) {
          store( next_symbol );
          skip_next( );
        }

        reset( ((next_symbol == 'L' || next_symbol == 'l') ? "long" : "int") + string(":") + stored( ) );
        return true; 
      }

    /* may find a number */
    default:
      // identify number
      if (isdigit( symbol )) {
        store( symbol );
        state = number;

        // try to identify a long integer immediately (possible)
        if (next_symbol == 'L' || next_symbol == 'l') {
          store( next_symbol );
          skip_next( );
          reset( "long:" + stored( ) );

          return true;
        } else if (!isdigit( next_symbol )) {
          reset( "int:" + stored( ) );
          return true;
        }
      }
  }

  return false;
}

bool scan_identifiers( char symbol, char next_symbol ) {
  switch (state) {
    case identifier:
      if (isalpha( symbol )) {
        store( symbol );

        if (!isalpha( next_symbol )) {
          if (keywords.find( stored( ) ) != keywords.end()) 
            reset( "keyword:" + stored( ) );
          else if (!isalnum( next_symbol )) 
            reset( "identifier:" + stored( ) );
          return true;
        }
        
        return false;
      }

    case undefined:
      if (isalpha( symbol ) || next_symbol == '_') {
        if (isalpha( next_symbol )) {
          store( symbol );
          state = identifier;
        } else reset( "identifier:" + string( 1, symbol ) );

        return true;
      }
  }

  return false;
}

bool scan_operators( char symbol, char next_symbol ) {
  if (operators.find( string( 1, symbol ) ) != operators.end()) {
    string match = string() + symbol + next_symbol;

    store( symbol );
    if (operators.find( match ) != operators.end()) {
      store( next_symbol );
      skip_next( );
    }

    reset( "operator:" + stored( ) );
    return true;
  }

  return false;
}

void reset( string msg ) {
  cache = "";
  state = undefined;

  if (msg.length() > 1) cout << msg << "\n";
}

void store( char in ) {
  cache += in;
}

string stored( ) {
  return cache;
}

void skip_next() {
  cin.ignore(1);
}

void skip_until( char c ) {
  cin.ignore(256, c);
}
