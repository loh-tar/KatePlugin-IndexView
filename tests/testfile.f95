! https://en.wikipedia.org/wiki/Fortran_95_language_features
! FIXME Improve this file with more meaningful content

FUNCTION string_concat(s1, s2)                             ! This is a comment
   TYPE (string), INTENT(IN) :: s1, s2
   TYPE (string) string_concat
   string_concat%string_data = s1%string_data(1:s1%length)&
      &s2%string_data(1:s2%length)                          ! This is a continuation
   string_concat%length = s1%length + s2%length
END FUNCTION string_concat

! Note the trailing comments and the trailing continuation mark.
! There may be 39 continuation lines, and 132 characters per line. Blanks are significant.
! Where a token or character constant is split across two lines:
! a leading & on the continued line is also required.
               ...        start_of&
        &_name
               ...   'a very long &
        &string'


INTEGER, PARAMETER :: two_bytes = SELECTED_INT_KIND(4)
! The forms of literal constants for CHARACTER data type are
'A string'   "Another"   'A "quote"'   '''''''
! (the last being an empty string)

! Derived data types
! For derived data types, the form of the type must be defined first:
TYPE person
   CHARACTER(10) name
   REAL          age
END TYPE person
! and then, variables of that type can be defined:
TYPE(person) you, me
! To select components of a derived type, % qualifier is used:
you%age

! Definitions may refer to a previously defined type:

TYPE point
   REAL x, y
END TYPE point
TYPE triangle
   TYPE(point) a, b, c
END TYPE triangle


INTERFACE OPERATOR(//) !Overloads the // operator as invoking string_concat procedure
  MODULE PROCEDURE string_concat
END INTERFACE





MODULE string_type
   IMPLICIT NONE
   TYPE string80
      INTEGER length
      CHARACTER(LEN=80)   :: string_data
   END TYPE string80
   INTERFACE ASSIGNMENT(=)
      MODULE PROCEDURE c_to_s_assign, s_to_c_assign
   END INTERFACE
   INTERFACE OPERATOR(//)
      MODULE PROCEDURE string_concat
   END INTERFACE
CONTAINS
   SUBROUTINE c_to_s_assign(s, c)
      TYPE (string80), INTENT(OUT)    :: s
      CHARACTER(LEN=*), INTENT(IN)  :: c
      s%string_data = c
      s%length = LEN(c)
   END SUBROUTINE c_to_s_assign
   SUBROUTINE s_to_c_assign(c, s)
      TYPE (string80), INTENT(IN)     :: s
      CHARACTER(LEN=*), INTENT(OUT) :: c
      c = s%string_data(1:s%length)
   END SUBROUTINE s_to_c_assign
   TYPE(string80) FUNCTION string_concat(s1, s2)
      TYPE(string80), INTENT(IN) :: s1, s2
      TYPE(string80) :: s
      INTEGER :: n1, n2
      CHARACTER(160) :: ctot
      n1 = LEN_TRIM(s1%string_data)
      n2 = LEN_TRIM(s2%string_data)
      IF (n1+n2 <= 80) then
         s%string_data = s1%string_data(1:n1)//s2%string_data(1:n2)
      ELSE  ! This is an error condition which should be handled - for now just truncate
         ctot = s1%string_data(1:n1)//s2%string_data(1:n2)
         s%string_data = ctot(1:80)
      END IF
      s%length = LEN_TRIM(s%string_data)
      string_concat = s
   END FUNCTION string_concat
END MODULE string_type

PROGRAM main
   USE string_type
   TYPE(string80) :: s1, s2, s3
   CALL c_to_s_assign(s1,'My name is')
   CALL c_to_s_assign(s2,' Linus Torvalds')
   s3 = s1//s2
   WRITE(*,*) 'Result: ',s3%string_data
   WRITE(*,*) 'Length: ',s3%length
END PROGRAM
