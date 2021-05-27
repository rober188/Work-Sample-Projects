#include<iostream>
#include<fstream>
#include<string>
#include<map>
using namespace std;

void story();
void connector();
void subject();
void verb();
void tense();
void be();
void noun();
void afternoun();
void object();
void afterobject();
void destination();
void period();
void gen(string);
void getEword();

/// globals ======================================
enum tokentype { 
  ERROR, EOFM, PERIOD, WORD1, WORD2, 
  VERB, VERBNEG, VERBPAST, VERBPASTNEG, 
  IS, WAS, PRONOUN, CONNECTOR, OBJECT, 
  SUBJECT, DESTINATION 
};
struct reservedWord {
  string word; // the word (string)
  tokentype type; // the type (tokentype)
  // constructor for struct that assigns word/type
  reservedWord(string w, tokentype t) {
    word = w;
    type = t;
  }
};

enum state{     
Q0,Q01,QY0,QC,QS,QSA,QT,QY,QERROR,
}; // no Q1 because there is no way to get to Q1

// For the display names of tokens - must be in the same order as the tokentype.
string tokenName[] = { 
  "ERROR", "EOFM", "PERIOD", "WORD1", "WORD2",
  "VERB", "VERBNEG", "VERBPAST", "VERBPASTNEG",
  "IS", "WAS", "PRONOUN", "CONNECTOR", "OBJECT",
  "SUBJECT", "DESTINATION"
}; 

// Reserved words table setup.
// we used an array of struct reservedWord rather than a string[][] because we wanted to easily access the enums from each reserved word
reservedWord reservedWords[] = {
  reservedWord("masu", VERB), // use reservedWord struct's constructor as array element
  reservedWord("masen", VERBNEG),
  reservedWord("mashita", VERBPAST),
  reservedWord("masendeshita", VERBPASTNEG),
  reservedWord("desu", IS),
  reservedWord("deshita", WAS),
  reservedWord("o", OBJECT),
  reservedWord("wa", SUBJECT),
  reservedWord("ni", DESTINATION),
  reservedWord("watashi", PRONOUN),
  reservedWord("anata", PRONOUN),
  reservedWord("kare", PRONOUN),
  reservedWord("kanojo", PRONOUN),
  reservedWord("sore", PRONOUN),
  reservedWord("mata", CONNECTOR),
  reservedWord("soshite", CONNECTOR),
  reservedWord("shikashi", CONNECTOR),
  reservedWord("dakara", CONNECTOR),
  reservedWord("eofm", EOFM)
};

map<string, string> lexicon;

tokentype saved_token;
string saved_lexme;
string saved_E_word;
string filename;
bool token_available;

// end globals ==================================

// --------- Two DFAs ---------------------------------

// WORD DFA 
// RE: (vowel | vowel n | consonant vowel | consonant vowel n | consonant-pair vowel | consonant-pair vowel n)^+
// enum for states in word DFA

bool word (string s)
{
  int i = 0;
  state state = Q0;
  char cur = s[i]; // set 'cur' character to first character of word
  /* replace the following todo the word dfa  **/
  while (cur != '\0') 
  {
    //cout << "|_|" << state << "|_|" << endl; was for testing
    switch(state) // switch statement based on case (looked better than if statements)
    {
      case Q0: case QY0:
        switch(cur) // switch statement based on 'cur' = current character (looked better than if statements)
        {
          case 'n': case 'h': case 'b': case 'm': case 'k': case 'p': case 'r': case 'g':
            state = QY;
            break;
          case 'y': case 'd': case 'w': case 'z': case 'j':
            state = QSA;
            break;
          case 'a': case 'i': case 'u': case 'e': case 'o': case 'E': case 'I': // vowels
            state = Q01;
            break;
          case 's':
            state = QS;
            break;
          case 'c':
            state = QC;
            break;
          case 't':
            state = QT;
            break;
          default: // default to QERROR if char is outside of language 
            state = QERROR;
            break;
        }
        break;
      case Q01:
        switch(cur)
        {
          case 'n':
            state = QY0;
            break;
          case 'h': case 'b': case 'm': case 'k': case 'p': case 'r': case 'g':
            state = QY;
            break;
          case 'y': case 'd': case 'w': case 'z': case 'j':
            state = QSA;
            break;
          case 'a': case 'i': case 'u': case 'e': case 'o': case 'E': case 'I': // vowels
            state = Q01;
            break;
          case 's':
            state = QS;
            break;
          case 'c':
            state = QC;
            break;
          case 't':
            state = QT;
            break;
          default:
            state = QERROR;
            break;
        }
        break;
      case QC:
        switch(cur)
        {
          case 'h':
            state = QSA;
            break;
          default:
            state = QERROR;
            break;
        }
        break;
      case QS:
        switch(cur)
        {
          case 'u': case 'a': case 'i': case 'e': case 'o': case 'E': case 'I': // vowels
            state = Q01;
            break;
          case 'h':
            state = QSA;
            break;
          default:
            state = QERROR;
            break;
        }
        break;
      case QSA:
        switch(cur) {
          case 'u': case 'a': case 'i': case 'e': case 'o': case 'E': case 'I': // vowels
            state = Q01;
            break;
          default:
            state = QERROR;
            break;
        }
        break;
      case QT:
        switch(cur) {
          case 'u': case 'a': case 'i': case 'e': case 'o': case 'E': case 'I': // vowels
            state = Q01;
            break;
          case 's':
            state = QSA;
            break;
          default:
            state = QERROR;
            break;
        }
        break;
      case QY:
        switch(cur) {
          case 'u': case 'a': case 'i': case 'e': case 'o': case 'E': case 'I': // vowels
            state = Q01;
            break;
          case 'y':
            state = QSA;
            break;
          default:
            state = QERROR;
            break;
        }
        break;
      default:
        state = QERROR;
        break;
    }
    i++;
    cur = s[i];
  }//end of while
  // where did I end up?
  return (state == Q01 || state == QY0); // return boolean where true for if you landed on a final state
}

// PERIOD DFA 
// RE: \.{1}
bool period (string s)
{
  return (s == "."); // return true if you've hit a period
}

// ------------ Scanner and Driver ----------------------- 

ifstream fin;  // global stream for reading from the input file
ofstream fout; // global stream for outputting translated.txt
// Scanner processes only one word each time it is called
// Gives back the token type and the word itself
void scanner(tokentype& tt, string& w)
{
  cout << "Scanner called using word: " << w << endl;
  // initizlie tt (token type)
  tt = ERROR;
  fin >> w;   // Grab the next word from the file via fin
  
  // 1. If it is eofm, then return.   
  if(w == "eofm") {  // check if word is end of file marker
    tt = EOFM;
    return;
  }  
  
  // 2. Call the token functions (word and period) 
  // one after another (if-then-else).
  // Generate a lexical error message if both DFAs failed.
  // Let the tokentype be ERROR in that case.
  // 3. If it was a word,
  // check against the reservedwords list.
  // If not reserved, tokentype is WORD1 or WORD2
  // decided based on the last character.
  if(word(w)){                                  // check if word is valid in word DFA
    for( reservedWord word : reservedWords){    // compare w string with reserved words
      if(w == word.word){                       // if w is a reserved word, give appropriate type (check against struct word value)
        tt = word.type;                         // since the word is in the reserved words list then match the type (via struct type value)
        //cout << w << "|_|" << word.word << "|_|" << word.type << endl; // test output
        return;
      }
    }
    // check for either word1/word2
    char last = w.back();
    if(last == 'a' || last == 'e' || last == 'i' || last == 'o' || last == 'u' || last == 'n')  tt = WORD1;  // check WORD1
    else if (last == 'E' || last == 'I') tt = WORD2;                                          // check WORD2
    else tt = ERROR;                             // if it is a word, but not WORD1 or WORD2 or a reservedWord then, type is ERROR
    return;       // exit function
  }
  else if(period(w)) tt= PERIOD; // token type period
  else { tt = ERROR; cout << "LEXICAL ERROR: " << w << " is not a valid token" << endl; } // error output
  return;
  // 4. Return the token type & string  (pass by reference)      
} //the end of scanner


// ================================================================ SCANNER END ================================================================//

// ----- Four Utility Functions and Globals -----------------------------------

// Type of error: expected token
void syntaxerror1(tokentype exp_token, string lexme) {
  cout << endl << "SYNTAX ERROR: expected " << tokenName[exp_token] << " but found " << lexme << " (" << tokenName[saved_token] << ")" << endl;
  fout << endl << "SYNTAX ERROR: expected " << tokenName[exp_token] << " but found " << lexme << " (" << tokenName[saved_token] << ")" << endl;
}

// Type of error: unexpected token
void syntaxerror2(tokentype component, string lexme) {
  cout << endl << "SYNTAX ERROR: unexpected " << lexme << " in " << tokenName[component] << endl;
  fout << endl << "SYNTAX ERROR: unexpected " << lexme << " in " << tokenName[component] << endl;
  exit(2);
}

// ** Need the updated match and next_token with 2 global vars
// saved_token and saved_lexeme

// Purpose: Obtainining the next token type
tokentype next_token() {

  if(!token_available) { //check to see if we should be consuming (depending on match)
    scanner(saved_token,saved_lexme); //save+consume next scanner input
    token_available = true; //need to check the token
  }

  return saved_token; //return with current token
}

// Purpose: Matching the expected token type
bool match(tokentype expected) {
  if (next_token() != expected) { // if you did not match
    // you have not matched the token, and will not consume more until matched
    return false; // no match
  } 
  else{ // you matched the token
    token_available = false; // you have consumed the token, so set available to false so scanner can consume again
    //cout << endl << "TOKEN: " << saved_lexme << " | " << tokenName[saved_token] << endl; // for testing
    return true; // matched!
  }

}

// ----- RDP functions - one per non-term -------------------


// Grammar: <story> :: <s> {<s>}
void story() {
  cout << "Processing <s>\n";
  if(match(CONNECTOR))
  {
    connector();
  }
  else if(match(PRONOUN))
  {
    noun();
  }
  else if(match(WORD1))
  {
    noun();
  }
  else {
    syntaxerror1(CONNECTOR, saved_lexme);
  }
}

// Grammar: <s> ::= [CONNECTOR #getEword# #gen(“CONNECTOR”)#] <noun> #getEword# SUBJECT 
void connector() {
  getEword();
  gen("CONNECTOR");

  cout << "Processing <connector>\n";
  if(match(PRONOUN))
  {
    noun();
  }
  else if(match(WORD1))
  {
    noun();
  }
  else {
    syntaxerror1(WORD1, saved_lexme);
  }
}

// Grammar: <subject> ::= <verb> <tense> PERIOD | <noun> {} | <afternoun> {}
void subject() {

  cout << "Processing <subject>\n";
  if(match(WORD1)) {
    afternoun();
  }
  else if(match(WORD2)) {
    verb();
  }
  else if(match(PRONOUN)) {
    afternoun();
  }
  else {
    syntaxerror1(WORD1, saved_lexme);
  }
}

// Grammar: <verb> ::= > #getEword# #gen(“ACTION”)# WORD2
void verb() {
  getEword();
  gen("ACTION");

  cout << "Processing <verb>\n";
  if(match(VERBPAST)){
    tense();
  }
  else if(match(VERBPASTNEG)) {
    tense();
  }
  else if(match(VERBNEG)) {
    tense();
  }
  else if(match(VERB)) {
    tense();
  }
  else {
    syntaxerror1(VERB, saved_lexme);
  }
}

// Grammar: <tense> ::= > #getEword# #gen(“TENSE”)# VERBPAST | VERBPASTNEG | VERB | VERBNEG
void tense() {
  getEword();
  gen("TENSE");

  cout << "Processing <tense>\n";
  if(match(PERIOD))
  {
    period();
  }
  else {
    syntaxerror1(PERIOD, saved_lexme);
  }
}

// Grammar: <be> ::= > #getEword# #gen(“TENSE”)# IS | WAS
void be() {
  getEword();
  gen("TENSE");

  cout << "Processing <be>\n";
  if(match(PERIOD))
  {
    period();
  }
  else {
    syntaxerror1(PERIOD,saved_lexme);
  }
}

// Grammar: <noun> ::= > #getEword# #gen(“ACTOR”)# WORD1 | PRONOUN
void noun() {
  getEword();
  gen("ACTOR");

  cout << "Processing <noun>\n";
  if(match(SUBJECT)) {
    subject();
  }
  else {
    syntaxerror1(SUBJECT, saved_lexme);
  }
}

// Grammar: <after noun> ::= #getEword# <be>PERIOD| DESTINATION<verb> <tense>| OBJECT<after object>
void afternoun() {
  getEword();
  

  cout << "Processing <afternoun>\n";
  if(match(IS)) {
    gen("DESCRIPTION");
    be();
  }
  else if(match(WAS)) {
    gen("DESCRIPTION");
    be();
  }
  else if(match(DESTINATION)) {
    gen("TO");
    destination();
  }
  else if(match(OBJECT)) {
    gen("OBJECT");
    object();
  }
  else {
    syntaxerror1(OBJECT,saved_lexme);
  }
}

// Grammar: OBJECT<verb><tense> PERIOD | OBJECT<noun>DESTINATION<verb><tense> PERIOD
void object() {

  cout << "Processing <object>\n";
  if(match(WORD1)) {
    afterobject();
  }
  else if(match(WORD2)) {
    verb();
  }
  else if(match(PRONOUN)) {
    afterobject();
  }
  else {
    syntaxerror1(WORD1, saved_lexme);
  }
}

// Grammar: <after object> ::= > #getEword# #gen("TO”)# <verb> <tense> PERIOD | <noun> DESTINATION <verb> <tense> PERIOD
void afterobject() {
  getEword();
  gen("TO");
  
  cout << "Processing <afterobject>\n";
  if(match(DESTINATION)) {
    destination();
  }
  else {
    syntaxerror1(DESTINATION, saved_lexme);
  }
}

// Grammar: <destination> ::= <verb> {}
void destination() {

  cout << "Processing <destination>\n";
  if(match(WORD2)) {
    verb();
  }
  else {
    syntaxerror1(WORD2,saved_lexme);
  }
}

// Grammar: <verb><tense> PEROD EOF
void period() {
  fout << endl;

  cout << "Processing <period>\n";
  if(match(CONNECTOR))
  {
    connector();
  }
  else if(match(PRONOUN))
  {
    noun();
  }
  else if(match(WORD1))
  {
    noun();
  }
  else if(match(EOFM)) {
    // done
    cout <<"done";
  }
  else
  {
    syntaxerror2(saved_token,saved_lexme);
  }
    
}

void getEword(){
  if(lexicon.count(saved_lexme)) // if there exists an english translation
  {
    saved_E_word = lexicon.at(saved_lexme); // set the E-word to the translation
  }
  else if(saved_token != WORD1) // else if it's NOT a noun
  {
    saved_E_word = tokenName[saved_token]; // set the E-word to the token type
  }
  else // else then it's a noun
  {
    saved_E_word = saved_lexme; // set the E-word to the saved jap word untranslated
  }

  // moved the saved_token/saved_lexme quandry from gen to here (above)
}

void gen(string line_type)
{
  // since saved_token/saved_lexme get's sorted into E-word if there is no translation
  fout << line_type << ": " << saved_E_word << endl; // just print the line type and E-word
}

void readLexicon() {
  string word, eng; // placeholder strings
  while(fin >> word) { // while there are still words in the lexicon
    fin >> eng; // assume that there will be an english definition for a word
    lexicon.insert(pair<string,string>(word, eng)); // insert the pair into the lexicon
  }

  cout << endl << "Lexicon: " << endl;

  cout << endl << "Successfully read lexicon!" << endl;
}

// ---------------- Driver ---------------------------

// The final test driver to start the translator
int main()
{

  //** opens the lexicon.txt file and reads it into Lexicon
  fin.open("lexicon.txt");
  if(!fin) { return 1; }  // error opening file
  readLexicon();          // read lexicon from file
  fin.close();            // closes lexicon.txt 
  
  fout.open("translated.txt",ios::app);  //** opens the output file translated.txt, and sets it to append
  // ^ implemented this way to quickly test multiple files
  
  cout << "Enter the input file name: ";
  cin >> filename;
  fin.open(filename.c_str());

  story();      // calls the <story> to start parsing
  
  fin.close();  // closes the input file 
  fout << "------------------- " << filename << ": END " << "-----------------------" << endl;
  fout.close(); // closes traslated.txt
}// end
