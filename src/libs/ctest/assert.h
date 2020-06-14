#pragma once

char assert_null(char *assertion, void *ptr);

char assert_int_equals(char *assertion, int num1, int num2);

char assert_int_less_than(char *assertion, int num1, int num2);

char assert_str_equals(char *assertion, char *str1, char *str2);

char assert_substr_in(char *assertion, char *str1, char *str2);

char assert_float_less_than(char *assertin, float n1, float n2);
 
char assert_double_equals(char *assertion, double num1, double num2);
