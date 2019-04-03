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
enum ScannerState {

  /* indeterminate state - nothing has been scanned */
  undefined,

  /* currently skipping over a comment */
  comment,

  /* reading an identifier, which may be a keyword instead */
  identifier,

  /* reading a normal or long integer */
  number,

  /* reading a string literal "" */
  text

};

/* symbols that signal comments */
const string COMMENT_CLOSE = "*/";
const string COMMENT_OPEN = "/*";
const string COMMENT_SINGLE = "//";

/* valid Simple C keywords */
const set<string> keywords {
  "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", 
  "float", "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", 
  "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

/* valid Simple C operators */
const set<string> operators {
  "=", "|", "||", "&&", "==", "!=", "<", ">", "<=", ">=", "+", "-", "*", "/", "%", "&", "!", "++", "--", ".", 
  "->", "(", ")", "[", "]", "{", "}", ";", ":", ","
};

/* scan input for each type of Simple C syntax
  returns true when a match is found with input symbol, false if the input symbol does not fit
  the conditions of this scan */
bool scan_comments( char& symbol, char& next_symbol );
bool scan_strings( char& symbol, char& next_symbol );
bool scan_numbers( char& symbol, char& next_symbol );
bool scan_identifiers( char& symbol, char& next_symbol );
bool scan_keywords( char& symbol, char& next_symbol );
bool scan_operators( char& symbol, char& next_symbol );

/* returns true when symbol matches L or l, indicating a long int */
bool is_long( char symbol );
/* returns true when symbol is a valid identifier character, i.e. underscores and alpha */
bool is_id( char symbol );
/* returns a string from one char input */
string chartos( char symbol );

/* empties the buffer, resets ScannerState to default and prints an output message */
void reset( string msg = "" );
/* store a character in the buffer */
void store( char in );
/* return the current buffer of characters */
string stored( );
/* skip the next character in the buffer, typically after peeking and deciding to toss it */
void skip_next( );
/* skip input until first character (c) is found */
void skip_until( char c );

/* string of increasing size as input is processed, returned to default ScannerState by reset( ) */
string cache = "";
ScannerState state = undefined;

int main( void ) {
  char c;

  while (cin.get( c )) {
    // always store next char in case we need it (likely)
    char peek = cin.peek( );

    // each method returns true if an item was processed, i.e., ready to move to next char
    if (scan_comments( c, peek )) continue;
    if (scan_strings( c, peek )) continue;
    if (scan_identifiers( c, peek )) continue;
    if (scan_operators( c, peek )) continue;
    if (scan_numbers( c, peek )) continue;
  }

  return EXIT_SUCCESS;
}

bool scan_comments( char& symbol, char& symbol_next ) {
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

bool scan_strings( char& symbol, char& next_symbol ) {
  switch (state) {
    case text:
      /* reached the end of the string */
      if (symbol == '"') 
        reset( "string:\"" + stored( ) + "\"" );
      /* otherwise continue reading string */
      else 
        store( symbol );

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

bool scan_numbers( char& symbol, char& next_symbol ) {
  switch (state) {
    // continuing to read a number
    case number:
      if (isdigit( next_symbol )) return true;
      /* reached the end of the number */
      else {
        store( symbol );
        
        if (is_long( next_symbol )) {
          store( next_symbol );
          skip_next( );
        }

        reset( (is_long( next_symbol ) ? "long" : "int") + string(":") + stored( ) );
        return true; 
      }

    // may find a number 
    case undefined:
      /* identify numbers */
      if (isdigit( symbol )) {
        store( symbol );
        state = number;

        // try to identify a one-character long int
        if (is_long( next_symbol )) {
          store( next_symbol );
          skip_next( );

          reset( "long:" + stored( ) );
          return true;
        } else if (!isdigit( next_symbol )) {
          /* or maybe we've matched a one-character int */
          reset( "int:" + stored( ) );
          return true;
        }

        return true;
      }
  }

  return false;
}

bool scan_identifiers( char& symbol, char& next_symbol ) {
  switch (state) {
    // continuining previous identifier
    case identifier:
      store( symbol );

      /* peek to make sure we haven't reached the end */
      if (!is_id( next_symbol )) {
        // if so, maybe it's a keyword
        if (keywords.find( stored( ) ) != keywords.end( )) 
          reset( "keyword:" + stored( ) );
        // otherwise, just an identifier
        else reset( "identifier:" + stored( ) );

        return true;
      }
      
      return false;

    // might be an indentifier
    case undefined:
      /* note we specifically prevent numbers from being first symbol */
      if (is_id( symbol ) && !isdigit( symbol )) {
        // identify continuing (longer than one char) identifier
        if (is_id( next_symbol )) {
          store( symbol );
          state = identifier;
        } else reset( "identifier:" + chartos( symbol ) );

        return true;
      }
  }

  return false;
}

bool scan_operators( char& symbol, char& next_symbol ) {
  /* try to find this symbol in the operators table */
  if (operators.find( chartos( symbol ) ) != operators.end()) {
    store( symbol );

    // try to match the next character as well, identifying operators like &&
    string match = chartos( symbol ) + chartos( next_symbol );
    if (operators.find( match ) != operators.end()) {
      store( next_symbol );
      skip_next( );
    }

    reset( "operator:" + stored( ) );
    return true;
  }

  return false;
}

bool is_long( char symbol ) {
  return symbol == 'L' || symbol == 'l';
}

bool is_id( char symbol ) {
  return isalpha( symbol ) || isdigit( symbol ) || symbol == '_';
}

string chartos( char symbol ) {
  return string( 1, symbol );
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
