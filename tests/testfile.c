// This testfile is (almost) as it was before the fork from SymbolViewer
// but has some note added and more stuff append to the end

/** Test File...very demoniac for parsing... **/
#include <stdio.h>
#include <stdlib.h>

#ifdef (_cplusplus)
// obviously a wrong placed brace
{
#define VALUE 5

#define MACRO(x)    (x^2)

// Is not recognised, see no need for that
// https://en.cppreference.com/w/cpp/preprocessor/replace
#    define abs_float(x)            \
          ( ((x)<0) ? -(x) : (x) )
              

typedef struct
   {
    pTest *pNext;
    pTest *pPrev;
   } 
   Another_test, *pTest;

typedef struct xauth
{
  unsigned short family;
  char *address;
} Xauth;

typedef struct {
    color    to_move;
    occupant board[8][8];
   } game;

typedef game  gt_data;
    
/* 
 A comment with a function hello() { } 
*/
// Continued...

RockType *
     MyMusicFunction(
          void *Red,
          int Hot, // Comment double slash
          char Chili, /* Comment inline */
          unsigned long Peppers)
// A comment..just to make some noise...
{
 // Passed first stage ???
 // Ok..get ready for the second one !
 if(I_Have_Failed() >= 0 && /* comments everywhere :} */
    This_Appears() == 1)
     {
      printf(QStringLiteral("Damn !!! Better going to bed :(("));
      if ( vs ) // here you are ;}
     activateSpace( vs->currentView() );
     } 
}

// Test escaped quote
void test() { printf("foo \"\n"); }
void test2() { printf("foo \"\n"); }

// Test tabs in macro
#define MY_MACRO(x) foo(x)
#define MY_MACRO2(x)	foo(x)
#define	MY_MACRO3(x)	foo(x)

// Test quoted characters ('"', '{', '}')
void quoteTest1() { if (token == '"') printf("foo\n"); }
void quoteTest2() { if (token == '{') printf("foo\n"); }
void quoteTest3() { if (token == '}') printf("foo\n"); }
void quoteTest4() { printf("foo\n"); }

foo bar /* Typical /*line at the beginning of /*some file
 * See COPYING for more details.
 */;

class SomeClass : public SomeParent
{
	SomeType someFunction1();
	SomeType someFunction2() some stuff;
}

class [[ using attribute-namespace : attribute-list ]] Cpp_Parser2 : public ProgramParser
{}

CppParser(IndexView *view)
//     : ProgramParser(view)
{}

~CppParser()
{}

void ProgramParser::addNode(const int nodeType, const QString &text, const int lineNumber)
{
    QTreeWidgetItem *node = nullptr;
}

// Object creation, looks similar to function declaration, must not be shown
KConfigGroup cfg(config, "General Options");
SomeClass foo(bar, baz);


// function returning a pointer to f0
auto fp11() -> void(*)(const std::string&)
{
    return f0;
}
 // function with an exception specification and a function try block
int f2(std::string str) noexcept try
{
    return std::stoi(str);
}
catch(const std::exception& e)
{
    std::cerr << "stoi() failed!\n";
    return 0;
}

QSize KWrite::sizeHint () const
{
}

static const char funcRetConstChar() {

}

// https://en.cppreference.com/w/c/language/typedef
struct tnode {
    int count;
    tnode *left, *right;
};
typedef struct {
	double hi, lo;
}
range;

typedef int int_t;
typedef struct tnode tnode;
typedef struct { double hi, lo; } range;
// FIXME These three are not properly handled...
typedef int arr_t[3];
typedef arr_t* (*fp)(void);
// ...and this is bad style for my taste
typedef char char_t, *char_p, (*fp)(void);

// https://en.cppreference.com/w/cpp/language/enum
enum Color1 { red, green, blue };
enum class Color2 { red, green = 20, blue };
enum smallenum: int16_t { a, b, c };
enum color3 { red, yellow, green = 20, blue };
enum class altitude: char { high='h', low='l',};
// Will not recognized, may be to FIXME
enum { d, e, f = e + 2 };

namespace IconCollection {
    void test() { printf("foo \"\n"); }
    enum IconType {
        FooType
    };
    QIcon getIcon(IconType type);
}

// Must shown...
class KATE_TESTS_EXPORT KateSessionsAction : public KActionMenu {}
void KTextEditor::ViewPrivate::home() {}
QMap<QString, QString> Speller::preferredDictionaries() const {}
// ...not sure if it must shown as function as it does currently, may be to FIXME
std::transform(sessionList.constBegin(), sessionList.constEnd(), std::back_inserter(sessionActions), [](const QString &session) {
	return QStringLiteral("Session %1").arg(QString::fromLatin1(QCryptographicHash::hash(session.toUtf8(), QCryptographicHash::Md5).toHex()));
});


// C++ has line continuation by backslash
// https://stackoverflow.com/a/7059563
#\
d\
e\
f\
i\
n\
e \
ABC \
int i

int main() {
A\
B\
C = 5;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
