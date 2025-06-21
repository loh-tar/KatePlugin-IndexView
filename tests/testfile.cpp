// This file contains all code snipplets from
// https://en.cppreference.com/w/cpp/language/class.html

class Vector; // forward declaration

class Matrix
{
    // ...
    friend Vector operator*(const Matrix&, const Vector&);
};

class Vector
{
    // ...
    friend Vector operator*(const Matrix&, const Vector&);
};

// In MyStruct.h
#include <iosfwd> // contains forward declaration of std::ostream

struct MyStruct
{
    int value;
    friend std::ostream& operator<<(std::ostream& os, const S& s);
    // definition provided in MyStruct.cpp file which uses #include <ostream>
};

struct s { int a; };
struct s; // does nothing (s already defined in this scope)

void g()
{
    struct s; // forward declaration of a new, local struct "s"
    // this hides global struct s until the end of this block

    s* p; // pointer to local struct s

    struct s { char* p; }; // definitions of the local struct s
}

class U;

namespace ns
{
    class Y f(class T p); // declares function ns::f and declares ns::T and ns::Y

    class U f(); // U refers to ::U

    // can use pointers and references to T and Y
    Y* p;
    T* q;
}

// 1) Member declarations
class S1
{
    int d1;             // non-static data member
    int a[10] = {1, 2}; // non-static data member with initializer (C++11)

    static const int d2 = 1; // static data member with initializer

    virtual void f1(int) = 0; // pure virtual member function

    std::string d3, *d4, f2(int); // FIXME two data members and a member function

    enum { NORTH, SOUTH, EAST, WEST };

    struct NestedS
    {
        std::string s;
    } d5, *d6;

    typedef NestedS value_type, *pointer_type;
};

// 2) Function definitions, which both declare and define member functions or friend functions.
class M1
{
    std::size_t C;
    std::vector<int> data;
public:
    M(std::size_t R, std::size_t C) : C(C), data(R*C) {} // constructor definition

    int operator()(std::size_t r, std::size_t c) const // member function definition
    {
        return data[r * C + c];
    }

    int& operator()(std::size_t r, std::size_t c) // another member function definition
    {
        return data[r * C + c];
    }
};

// 3) Access specifiers public:, protected:, and private:
class S2
{
public:
    S();          // public constructor
    S(const S&);  // public copy constructor
    virtual ~S(); // public virtual destructor
private:
    int* ptr; // private data member
};

// 4) Using-declarations:
class Base1
{
protected:
    int d;
};

class Derived1 : public Base2
{
public:
    using Base::d;    // make Base's protected member d a public member of Derived
    using Base::Base; // inherit all bases' constructors (C++11)
};

// 5) static_assert declarations:
template<typename T>
struct Foo
{
    static_assert(std::is_floating_point<T>::value, "Foo<T>: T must be floating point");
};

// 6) member template declarations:
struct S3
{
    template<typename T>
    void f(T&& n);

    template<class CharT>
    struct NestedS
    {
        std::basic_string<CharT> s;
    };
};

// 7) alias declarations:
template<typename T>
struct identity
{
    using type = T;
};


// 8) deduction guides of member class templates:
struct S4
{
    template<class CharT>
    struct NestedS
    {
        std::basic_string<CharT> s;
    };

    template<class CharT>
    NestedS(std::basic_string<CharT>) -> NestedS<CharT>;
};

// 9) Using-enum-declarations:
enum class color { red, orange, yellow };

struct highlight
{
    using enum color;
};

#include <algorithm>
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> v{1, 2, 3};

    struct Local
    {
        bool operator()(int n, int m)
        {
            return n > m;
        }
    };

    std::sort(v.begin(), v.end(), Local()); // since C++11

    for (int n : v)
        std::cout << n << ' ';
    std::cout << '\n';
}

// Here more code from
// https://en.cppreference.com/w/cpp/language/declarations.html
#include <type_traits>
struct Ss
{
    int member;
    // decl-specifier-seq is "int"
    // declarator is "member"
} obj, *pObj(&obj);// FIXME
// decl-specifier-seq is "struct S { int member; }"
// declarator "obj" declares an object of type S
// declarator "*pObj" declares a pointer to S,
//     and initializer "(&obj)" initializes it

// NOTE: Variables need a parent like namespace
// or will not be shown in our tree
namespace foo {
QHash<int, NodeTypeStruct>      p_nodeTypes;
QHash<int, QTreeWidgetItem*>    p_rootNodes;
std::size_t C;
std::vector<int> data;

char c = 'x';
int i;
int j = 1;
int k(42); // FIXME
const int i = 1;
int *p = nullptr;
int (*pf)(double); // FIXME

int i = 1, *p = nullptr, f(), (*pf)(double);// FIXME only i is found, not the rest
// decl-specifier-seq is "int"
// declarator "i" declares a variable of type int,
//     and initializer "= 1" initializes it
// declarator "*p" declares a variable of type int*,
//     and initializer "= nullptr" initializes it
// declarator "f()" declares (but doesn't define)
//     a function taking no arguments and returning int
// declarator "(*pf)(double)" declares a pointer to function
//     taking double and returning int

int (*(*var1)(double))[3] = nullptr;// FIXME
// decl-specifier-seq is "int"
// declarator is "(*(*var1)(double))[3]"
// initializer is "= nullptr"

// 1. declarator "(*(*var1)(double))[3]" is an array declarator:
//    Type declared is: "(*(*var1)(double))" array of 3 elements
// 2. declarator "(*(*var1)(double))" is a pointer declarator:
//    Type declared is: "(*var1)(double)" pointer to array of 3 elements
// 3. declarator "(*var1)(double)" is a function declarator:
//    Type declared is: "(*var1)" function taking "(double)",
//    returning pointer to array of 3 elements.
// 4. declarator "(*var1)" is a pointer declarator:
//    Type declared is: "var1" pointer to function taking "(double)",
//    returning pointer to array of 3 elements.
// 5. declarator "var1" is an identifier.
// This declaration declares the object var1 of type "pointer to function
// taking double and returning pointer to array of 3 elements of type int"
// The initializer "= nullptr" provides the initial value of this pointer.

// C++11 alternative syntax:
auto (*var2)(double) -> int (*)[3] = nullptr;// FIXME
// decl-specifier-seq is "auto"
// declarator is "(*var2)(double) -> int (*)[3]"
// initializer is "= nullptr"

// 1. declarator "(*var2)(double) -> int (*)[3]" is a function declarator:
//    Type declared is: "(*var2)" function taking "(double)", returning "int (*)[3]"
// ...
}

int main()
{
    static_assert(std::is_same_v<decltype(var1), decltype(var2)>);
}
