#!/bin/bash
#
# Pointless code but anyway helpful to test the Bash-Parser
#
someFunction() {
  local inTerminal="0"
  local -n txt="$2"
  # The # in the assignment must not catched as comment
  local txtWidth="${#txt}"
  local copy

  nestedFunction1() {
    copy="$txt"
    copy="${copy//[$'\r']}"
    xcopy="${copy//${clearLine}}"

    nestedNestedFunction1() {
      echo foo
      }

    txtWidth="${#copy}"
    }

  nestedFunction2() {
    nestedNestedFunction2() {
    }
  }

  # Don't show bad fuction name
  nestedO-ther2Function() {

    copy="${copy//[$'\n']}"
    txtWidth="${#copy}"
  }
  1
}

# TODO Improve this file to be less pointless
# but keep the showcase
function some_Other_Function() {
	function nested_Other_Function () {
	nestedNestedOtherFunction ()
	}
}
# TODO Improve the Bash-Parser with more options, not only functions
# Ideas?

cat << EOF > /some/file
These contents will be written to the file.
	function DontShowThisFunction() {
	}
        This line is indented.
EOF

# Here string may cause trouble
if grep -q "txt" <<< "$VAR"
then   #         ^^^
   echo "$VAR contains the substring sequence \"txt\""
fi

case ${#c6[*]} in
    3) (( c8[1] = ( (c6[1] & 15) << 4) | (c6[2] >> 2) ))
       (( c8[2] = (c6[2] & 3) << 6 )); unset c8[2] ;;
    4) (( c8[1] = ( (c6[1] & 15) << 4) | (c6[2] >> 2) ))
       (( c8[2] = ( (c6[2] &  3) << 6) |  c6[3] )) ;;
  esac

printf "%x\n" $(( 5 << 63 )) $(( 5 << 64 ))



#!/bin/bash

################ FLOWCHART STEPS 2 - 4 ####################

${awk} '{ print }' << 'AWKAWKPREPROC' > "${f100}"
BEGIN {                   <== DONT SHOW THIS BEGIN
  eex = "BEGIN {\n"       <== or that                                                  \
        "  error_exit_in_progress = 0\n"                                    \
        "}\n"                                                               \
  war = "function warning( msg ) {\n"                                       \
        "  if ( 0 == error_exit_in_progress ) {\n"                          \
        "    gsub( CNTRLREGEX, TRIPLETC, msg )\n"                           \
        "    print \"\\nZaloha AWK: Warning: \" msg > \"/dev/stderr\"\n"    \
        "  }\n"                                                             \
        "}"
  mpa = 8     # MAXPARALLEL constant
}
{
  gsub( /DEFINE_ERROR_EXIT/, eex )
  if ( $0 ~ /ONE_TO_MAXPARALLEL/ ) {
    for ( i = 1; i <= mpa; i ++ ) {
      s = $0
      gsub( /ONE_TO_MAXPARALLEL/, i, s )
      gsub( /MAXPARALLEL/, mpa, s )
      print s
    }
  }
}
AWKAWKPREPROC

after_heredoc_func() {
}
