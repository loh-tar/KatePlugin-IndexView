<!--
This odd test file has contend grabbed, and sometimes modified, from
https://secure.php.net/manual/en/langref.php

NOTE That FIXME/TODO is at this place not extracted because it's HTML comment
and not PHP one
-->

<?php namespace bar;
  class Cat {
    static function says() {echo 'meoow';}  } ?>

<?php
const ONE = 1;
define("FOO",     "something");
# Defining and using a constant
class MyClassA
{
    const CONSTANT = 'constant value';

    final function showConstant() {
        echo  self::CONSTANT . "\n";
    }
}

echo MyClassA::CONSTANT . "\n";

$classname = "MyClassA";
echo $classname::CONSTANT . "\n"; // As of PHP 5.3.0

$xclass = new MyClassA();
$xclass->showConstant();

echo $xclass::CONSTANT."\n"; // As of PHP 5.3.0
?>


<?php
# Nowdoc and HereDoc
# see also https://wiki.php.net/rfc/flexible_heredoc_nowdoc_syntaxes
class fooA {
    const BAR = <<<'EOT'
foo
    $dontShowThis1variable
    function dontShowThis1Function() {
    }
    FIXME Don't show this text
bar
EOT;

    const BAZ = <<<EOT
baz
    // Well, perhaps should $dontShowThis2variable be visible?
    $dontShowThis2variable
    function dontShowThis2Function() {
        echo $dontShowThis2variable
    }
faz
EOT;
}
?>

<?php

// FIXME Namespace stuff is more complex
// https://secure.php.net/manual/en/language.namespaces.definitionmultiple.php

# Namespaced ::class example
namespace foo {
    // Class bar is below foo namespace, where it should be, I think
    class bar {
    }

    echo bar::class; // foo\bar
}
?>


<?php

// Recognised as class but I think it's OK
stupid stuff class WrongKeyWords stupid stuff {
    const TWO = ONE * 2;
    const THREE = ONE + self::TWO;
    const SENTENCE = 'The value of THREE is '.self::THREE;
}

?>


<?php
# Class constant visibility modifiers
class Foo {
    // As of PHP 7.1.0
    public const BAR = 'bar';
    private const BAZ = 'baz';
}
echo Foo::BAR, PHP_EOL;
echo Foo::BAZ, PHP_EOL;
?>


<?php
# Final methods example
class BaseClass {
   public function test() {
       echo "BaseClass::test() called\n";
   }

   final public function moreTesting() {
       echo "BaseClass::moreTesting() called\n";
   }
}

class ChildClass extends BaseClass {
   public function moreTesting() {
       echo "ChildClass::moreTesting() called\n";
   }
}
// Results in Fatal error: Cannot override final method BaseClass::moreTesting()
?>


<?php
// Multiple interface inheritance
interface a
{
    public function foo();
}

interface b
{
    public function bar();
}

interface c extends a, b
{
    public function baz();
}

class d implements c
{
    public function foo()
    {
    }

    public function bar()
    {
    }

    public function baz()
    {
    }
}
?>


function youShouldNotSeeThis() {}


<?php

    echo 'This is a test'; // This is a one-line c++ style comment
    /* This is a multi line comment

function dontShowThis() {
<!--     # ...because it's not inside of a <\?php tag -->
}
       yet another line of comment */
    echo 'This is yet another test';
    echo 'One Final Test'; # This is a one-line shell-style comment
?>


<?php
// 'C' style comments end at the first */ encountered.
// Make sure you don't nest 'C' style comments.
// It is easy to make this mistake if you are trying to comment out a large block of code.

 /*
    echo 'This is a test'; /* This comment will cause a problem */
    function youShouldSeeThis() {}

*/ // <- That was intendet to end the block
?>


It is worth mentioning that, HTML comments have no meaning in PHP parser. So,

<!-- comment
<?php echo some_function(); ?>
-->

WILL execute some_function() and echo result inside HTML comment.
<!-- https://php.net/manual/en/language.basic-syntax.comments.php#113438 -->



function dontShowThis() {
<!--     # ...because it's not inside of a <\?php tag -->
}


<?php
// https://secure.php.net/manual/en/control-structures.switch.php
switch ($i) {
    case "apple":
        echo "i is apple";
        break;
    case "cake":
        echo "i is cake";
        break;
}

switch ($i):
    case 0:
        echo "i equals 0";
        break;
    case 2:
        echo "i equals 2";
        break;
    default:
        echo "i is not equal to 0 or 2";
endswitch;
?>

<!-- kate: space-indent on; indent-width 4; replace-tabs on; -->
