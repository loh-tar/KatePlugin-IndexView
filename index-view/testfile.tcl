#!/usr/bin/env tclsh
#
# I took parts from there to fill this test file
# https://en.wikibooks.org/wiki/Tcl_Programming/Introduction

# This is a comment \
going over four lines \
with backslash continuation and FIXME recognition \
set dontShowThis1 Text


# One of the problems new users of Tcl meet sooner or later is that comments
# behave in an unexpected way. For example, if you comment out part of code like this:

# if {$condition} {
    set foo "condition met!"
# }

# This happens to work, but any unbalanced braces in comments may lead to unexpected syntax errors.
# The reason is that Tcl's grouping (determining word boundaries) happens before the # characters are considered.

# To add a comment behind a command on the same line, just add a semicolon:
puts "this is the command" ;# that is the comment

# Comments are only taken as such where a command is expected.
# In data (like the comparison values in switch), a # is just a literal character:
if $condition {# good place
   switch -- $x {
       #bad_place {because switch tests against it}
       some_value {do something; # good place again}
       # NOTE Braces are now treated as comment terminator, thats not how Tcl works
       # but seams to work so far here
   }
}

# In contrast to many other languages, strings in Tcl don't need quotes for markup.
# The following is perfectly valid:
set greeting Hello!
# FIXME Not quoted strings are not considered

# In source code, quoted or braced strings can span multiple lines,
# and the physical newlines are part of the string too:
# FIXME Braced strings are not supported
set test "hello \
world
proc dontShowThis2 function {
}
or this FIXME text
"

if 0 {
   This is somehow strange Tcl multiline comment
   proc dontShowThis3 function {
   }
   but show this FIXME note
}


# DustMotePlus - with a subset of CGI support
set root      c:/html
set default   index.htm
set port      80
set encoding  iso8859-1

proc bgerror msg {puts stdout "bgerror: $msg\n$::errorInfo"}

proc answer {sock host2 port2} {
    fileevent $sock readable [list serve $sock]
    set foo bar
    set bar foo
}

proc serve sock {
    fconfigure $sock -blocking 0
    gets $sock line
    if {[fblocked $sock]} {
        return
    }
    fileevent $sock readable ""
    set tail /
    regexp {(/[^ ?]*)(\?[^ ]*)?} $line -> tail args
    if {[string match */ $tail]} {
        append tail $::default
    }
    set name [string map {%20 " " .. NOTALLOWED} $::root$tail]
    if {[file readable $name]} {
        puts $sock "HTTP/1.0 200 OK"
        if {[file extension $name] eq ".tcl"} {
            set ::env(QUERY_STRING) [string range $args 1 end]
            set name [list |tclsh $name]
        } else {
            puts $sock "Content-Type: text/html;charset=$::encoding\n"
        }
        set inchan [open $name]
        fconfigure $inchan -translation binary
        fconfigure $sock   -translation binary
        fcopy $inchan $sock -command [list done $inchan $sock]
    } else {
        puts $sock "HTTP/1.0 404 Not found\n"
        close $sock
    }
}

# FIXME Don't show 'file' as parameter, nor 'sock bytes'
proc done {file sock bytes {msg {}}} {
	proc nest1 {file sock bytes {msg {}}} {
	set foo bar
	}
	proc nest2 {file sock bytes {msg {}}} {
	set foo bar
	}
    close $file
    close $sock
}
set baz faz
socket -server answer $port
puts "Server ready..."
vwait forever


proc makeEnum {name values} {
   interp alias {} $name: {} lsearch $values
   interp alias {} $name@ {} lindex $values
}

# That's why "Bracket Commands" treated like strings
puts "Long date: [clock format $secs -format "%A, %B %d, %Y"]"
