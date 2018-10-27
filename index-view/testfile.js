// TODO Improve this haphazardly test file. Parts are taken from
// https://en.wikipedia.org/wiki/JavaScript

// A simple recursive function:
function factorial(n) {
    if (n === 0) {
        return 1;  // 0! = 1
    }
    return n * factorial(n - 1);
}

factorial(3); // returns 6



// An anonymous function (or lambda):
function counter() {
    var count = 0;
    return function() {
        return ++count;
    };
}

var closure = counter();
closure(); // returns 1
closure(); // returns 2
closure(); // returns 3



// Object example:
function Ball(r) {
    this.radius = r; //the radius variable is local to the ball object
    this.area = pi*r**2;
    this.show = function(){ //objects can contain functions
        drawCircle(r); //references a circle drawing function
    }
}
myBall = new Ball(5); //creates a new instance of the ball object with radius 5
myBall.show(); //this instance of the ball object has the show function performed on it



// More advanced example
/* Finds the lowest common multiple (LCM) of two numbers */
function LCMCalculator(x, y) { // constructor function
  var checkInt = function(x) { // inner function
    if (x % 1 !== 0) {
      throw new TypeError(x + "is not an integer"); // var a =  mouseX
    };

    return x
  }
  this.a = checkInt(x)
    //   semicolons   ^^^^  are optional, a newline is enough
  this.b = checkInt(y);

}
// The prototype of object instances created by a constructor is
// that constructor's "prototype" property.
LCMCalculator.prototype = { // object literal
  constructor: LCMCalculator, // when reassigning a prototype, set the constructor property appropriately
  gcd: function() { // method that calculates the greatest common divisor
    // Euclidean algorithm:
    var a = Math.abs(this.a),
      b = Math.abs(this.b),
      t;
    if (a < b) {
      // swap variables
      // t = b; b = a; a = t;
      [a, b] = [b, a]; // swap using destructuring assignment (ES6)
    }
    while (b !== 0) {
      t = b;
      b = a % b;
      a = t;
    }
    // Only need to calculate GCD once, so "redefine" this method.
    // (Actually not redefinition—it's defined on the instance itself,
    // so that this.gcd refers to this "redefinition" instead of LCMCalculator.prototype.gcd.
    // Note that this leads to a wrong result if the LCMCalculator object members "a" and/or "b" are altered afterwards.)
    // Also, 'gcd' === "gcd", this['gcd'] === this.gcd
    this['gcd'] = function() {
      return a;
    };
    return a;
  },
  // Object property names can be specified by strings delimited by double (") or single (') quotes.
  lcm: function() {
    // Variable names don't collide with object properties, e.g., |lcm| is not |this.lcm|.
    // not using |this.a*this.b| to avoid FP precision issues
    var lcm = this.a / this.gcd() * this.b;
    // Only need to calculate lcm once, so "redefine" this method.
    this.lcm = function() {
      return lcm;
    };
    return lcm;
  },
  toString: function() {
    return "LCMCalculator: a = " + this.a + ", b = " + this.b;
  }
};

// Define generic output function; this implementation only works for Web browsers
function output(x) {
  document.body.appendChild(document.createTextNode(x));
  document.body.appendChild(document.createElement('br'));
}

// Note: Array's map() and forEach() are defined in JavaScript 1.6.
// They are used here to demonstrate JavaScript's inherent functional nature.
[
  [25, 55],
  [21, 56],
  [22, 58],
  [28, 56]
].map(function(pair) { // array literal + mapping function
    return new LCMCalculator(pair[0], pair[1]);
  }).sort((a, b) => a.lcm() - b.lcm()) // sort with this comparative function; => is a shorthand form of a function, called "arrow function"
  .forEach(printResult);

function printResult(obj) {
  output(obj + ", gcd = " + obj.gcd() + ", lcm = " + obj.lcm());
}


(function foo() {
  var x = 7;
  console.log("val " + eval("x + 2"));
})();
