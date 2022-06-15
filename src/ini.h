#if defined(INI_IMPL) && !defined(INI_IMPLEMENTATION)
#define INI_IMPLEMENTATION
#endif
#ifndef INI_INCLUDED
/*
    ini.h -- a *tiny* library for reading an INI file.

    Project URL: https://github.com/mathewmariani/ini.h

    Do this:
      #define INI_IMPL or
      #define INI_IMPLEMENTATION

    before you include this file in *one* C/C++ file to create the
    implementation.

    ...optionally you can provide the following macros to override defaults:

    INI_ASSERT(c)     - your own assert function (default: assert(c))
    INI_MALLOC(s)     - your own malloc function (default: malloc(s))
    INI_FREE(p)       - your own free function (default: free(p))


    FEATURE OVERVIEW:
    =================

    ini.h provides a minimalistic API which implements
    the basics functions for reading an INI file.

    as the INI file format is not rigidly defined,
    the following is a list of some common features implemented:

    Feature             | Support |
    --------------------+---------+
    Read                | YES     |
    Write               | TODO    |
    Sections            | YES     |
    Section Nesting     | NO      |
    Properties          | YES     |
     - delimiter (=)    | YES     |
     - delimiter (:)    | NO      |
    Global Properties   | YES     |
    Disabled Properties | YES     |
    String Values       | YES     |
    Integer Values      | YES     |
    Float Values        | YES     |
    Boolean Values      | YES     |
    Quoted Values       | TODO    |
    Multi-line          | NO      |
    Comments (;)        | YES     |
    Comments (#)        | YES     |
    Escape Characters   | TODO    |


    FUNCTIONS:
    ==========

    ini_create()
    ini_destroy(ini_t* ini)
    ini_load(const char* data)
    ini_find_section(const ini_t* ini, const char* name)
    ini_section_exists(const ini_t* ini, const char* key)
    ini_property_exists(const ini_t* ini, int section, const char* key)
    ini_value(const ini_t* ini, int section, const char* key)
    ini_value_as_int(const ini_t* ini, int section, const char* key)
    ini_value_as_float(const ini_t* ini, int section, const char* key)
    ini_value_as_bool(const ini_t* ini, int section, const char* key)


    LICENSE:
    ========

    MIT License

    Copyright (c) 2022 Mat Mariani

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#define INI_INCLUDED (1)

#include <stdbool.h>

#if !defined(INI_API_DECL)
  #define INI_API_DECL extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* compile-time constants */
enum {
  INI_NOT_FOUND      = -1,
  INI_GLOBAL_SECTION = 0,
};

typedef struct ini_t ini_t;

/* create ini structure */
INI_API_DECL ini_t* ini_create();
/* destroy and cleanup ini structure */
INI_API_DECL void ini_destroy(ini_t* ini);
/* parses null terminated string containing an ini-file */
INI_API_DECL ini_t* ini_load(const char* data);
/* get index of a section; returns -1 if not found */
INI_API_DECL int ini_find_section(const ini_t* ini, const char* name);
/* tests if a section exists */
INI_API_DECL bool ini_section_exists(const ini_t* ini, const char* key);
/* tests if a property exists in a given section */
INI_API_DECL bool ini_property_exists(const ini_t* ini, int section, const char* key);
/* get value of propery in a given section; returns NULL if key doesn't exist */
INI_API_DECL const char* ini_value(const ini_t* ini, int section, const char* key);
/* get value of propery in a given section, as an int */
INI_API_DECL int ini_value_as_int(const ini_t* ini, int section, const char* key);
/* get value of propery in a given section, as a float */
INI_API_DECL float ini_value_as_float(const ini_t* ini, int section, const char* key);
/* get value of propery in a given section, as a boolean */
INI_API_DECL bool ini_value_as_bool(const ini_t* ini, int section, const char* key);

#ifdef __cplusplus
}

/* reference-based equivalents for c++ */
inline int ini_find_section(const ini_t& ini, const char* name) { return ini_find_section(&ini, name); }
inline bool ini_section_exists(const ini_t& ini, const char* key) { return ini_section_exists(&ini, key); }
inline bool ini_property_exists(const ini_t& ini, int section, const char* key) { return ini_property_exists(&ini, section, key); }
inline const char* ini_value(const ini_t& ini, int section, const char* key) { return ini_value(&ini, section, key); }
inline int ini_value_as_int(const ini_t& ini, int section, const char* key) { return ini_value_as_int(&ini, section, key); }
inline float ini_value_as_float(const ini_t& ini, int section, const char* key) { return ini_value_as_float(&ini, section, key); }
inline bool ini_value_as_bool(const ini_t& ini, int section, const char* key) { return ini_value_as_bool(&ini, section, key); }

#endif
#endif /* INI_INCLUDED */

#ifdef INI_IMPLEMENTATION
#define INI_IMPL_INCLUDED (1)

#include <string.h>

#if !defined(INI_ASSERT)
  #include <assert.h>
  #define INI_ASSERT(c) assert(c)
#endif
#if !defined(INI_MALLOC)
  #include <stdlib.h>
  #define INI_MALLOC(s) malloc(s)
  #define INI_FREE(p)   free(p)
#endif

#ifndef _INI_PRIVATE
  #if defined(__GNUC__) || defined(__clang__)
    #define _INI_PRIVATE __attribute__((unused)) static
  #else
    #define _INI_PRIVATE static
  #endif
#endif

#define INI_INITIAL_CAPACITY (256)
#define _INI_BUF_SIZE_DEF (16*1024)

/* private implementation functions */

typedef struct {
  int name;
} _ini_section_t;

typedef struct {
  int section;
  int key;
  int value;
} _ini_property_t;

struct ini_t {
  _ini_section_t* sections;
  _ini_property_t* properties;
  int num_sections;
  int num_properties;
  char* buf;
};

/* parser state */
enum {
  _INI_EXPECT_NONE = (0 << 0),
  _INI_EXPECT_KEY =  (1 << 0),
  _INI_EXPECT_SEP =  (1 << 1),
  _INI_EXPECT_VAL =  (1 << 2),
  _INI_PARSING_KEY = (1 << 3),
  _INI_PARSING_VAL = (1 << 4),
};

_INI_PRIVATE inline bool _ini_any_expected(int state) {
  const int flags = (_INI_EXPECT_KEY | _INI_EXPECT_VAL | _INI_EXPECT_SEP);
  return (state & (flags)) != _INI_EXPECT_NONE;
}

_INI_PRIVATE inline bool _ini_key_expected(int state) {
  return (state & _INI_EXPECT_KEY) != _INI_EXPECT_NONE;
}

_INI_PRIVATE inline bool _ini_val_expected(int state) {
  return (state & _INI_EXPECT_VAL) != _INI_EXPECT_NONE;
}

_INI_PRIVATE inline bool _ini_parsing_key(int state) {
  return (state & _INI_PARSING_KEY) != _INI_EXPECT_NONE;
}

_INI_PRIVATE inline bool _ini_parsing_val(int state) {
  return (state & _INI_PARSING_VAL) != _INI_EXPECT_NONE;
}

_INI_PRIVATE inline bool _ini_is_bracket(char c) {
  return ((c == '[') || (c == ']'));
}

_INI_PRIVATE inline bool _ini_is_whitespace(char c) {
  return ((c == ' ') || (c == '\t'));
}

_INI_PRIVATE inline bool _ini_is_delimeter(char c) {
  return (c == '=');
}

_INI_PRIVATE inline bool _ini_is_comment(char c) {
  return ((c == ';') || (c == '#'));
}

_INI_PRIVATE inline bool _ini_is_newline(char c) {
  return (c == '\n');
}

_INI_PRIVATE inline void _ini_start_section(ini_t* ini, int pos, int* state) {
  *state = _INI_PARSING_KEY;
  ini->sections[ini->num_sections].name = pos;
}

_INI_PRIVATE inline void _ini_start_key(ini_t* ini, int pos, int* state) {
  *state = _INI_PARSING_KEY;
  ini->properties[ini->num_properties].section = ini->num_sections;
  ini->properties[ini->num_properties].key = pos;
}

_INI_PRIVATE inline void _ini_start_val(ini_t* ini, int pos, int* state) {
  *state = _INI_PARSING_VAL;
  ini->properties[ini->num_properties].value = pos;
}

_INI_PRIVATE inline const char* _ini_str(const ini_t* ini, int index) {
  return &ini->buf[index];
}

_INI_PRIVATE ini_t* _ini_parse_data(const char* src) {
  int parser_state = _INI_EXPECT_KEY;
  bool in_bracket = false;
  int buf_pos = 0;
  ini_t* ini = ini_create();
  
  char c;
  while ((c = *src++) != 0) {
    /* handle comment: */
    if (_ini_is_comment(c)) {
      while (!_ini_is_newline(c)) {
        c = *src++;
      }
      continue;
    }
    /* get expected: */
    if (_ini_any_expected(parser_state)) {
      if (_ini_is_whitespace(c)) {
        continue;
      }
      if (_ini_key_expected(parser_state)) {
        if (_ini_is_bracket(c)) {
          in_bracket = true;
          _ini_start_section(ini, buf_pos, &parser_state);
          continue;
        }
        _ini_start_key(ini, buf_pos, &parser_state);
      }
      else if (_ini_val_expected(parser_state)) {
        _ini_start_val(ini, buf_pos, &parser_state);
      }
      else {
        if (_ini_is_delimeter(c)) {
          parser_state = _INI_EXPECT_VAL;
          continue;
        }
      }
    }
    /* begin parsing: */
    if (_ini_parsing_key(parser_state)) {
      if (in_bracket) {
        if (_ini_is_bracket(c)) {
          in_bracket = false;
          ini->buf[buf_pos++] = '\0';
          ini->num_sections++;
          parser_state = _INI_EXPECT_KEY;
          continue;
        }
      }
      if (_ini_is_whitespace(c) || _ini_is_delimeter(c)) {
        ini->buf[buf_pos++] = '\0';
        parser_state = _ini_is_delimeter(c) ? _INI_EXPECT_VAL : _INI_EXPECT_SEP;
        continue;
      }
    }
    else if (_ini_parsing_val(parser_state)) {
      if (_ini_is_newline(c)) {
        ini->buf[buf_pos++] = '\0';
        ini->num_properties++;
        parser_state = _INI_EXPECT_KEY;
        continue;
      }
    }
    /* handle newline: */
    if (_ini_is_newline(c)) {
      continue;
    }
    ini->buf[buf_pos++] = c;
  }
  if (_ini_parsing_val(parser_state)) {
    ini->buf[buf_pos++] = '\0';
    ini->num_properties++;
  }
  return ini;
}

/* public api functions */

ini_t* ini_create() {
  ini_t* ini = INI_MALLOC(sizeof(ini_t));
  ini->sections = INI_MALLOC(INI_INITIAL_CAPACITY * sizeof(_ini_section_t));
  ini->num_sections = 0;
  ini->properties = INI_MALLOC(INI_INITIAL_CAPACITY * sizeof(_ini_property_t));
  ini->num_properties = 0;
  ini->buf = INI_MALLOC(_INI_BUF_SIZE_DEF * sizeof(char));
  return ini;
}

void ini_destroy(ini_t* ini) {
  INI_ASSERT(ini);
  INI_FREE(ini->properties);
  INI_FREE(ini->sections);
  INI_FREE(ini->buf);
  INI_FREE(ini);
}

ini_t* ini_load(const char* data) {
  INI_ASSERT(data);
  return _ini_parse_data(data);
}

int ini_find_section(const ini_t* ini, const char* name) {
  INI_ASSERT(ini && name);
  _ini_section_t* section = &ini->sections[0];
  for (int i = 0; i < ini->num_sections; i++, section = &ini->sections[i]) {
    if (strcmp(_ini_str(ini, section->name), name) == 0) {
      return i+1;
    }
  }
  return INI_NOT_FOUND;
}

bool ini_section_exists(const ini_t* ini, const char* name) {
  return ini_find_section(ini, name) != INI_NOT_FOUND;
}

bool ini_property_exists(const ini_t* ini, int section, const char* key) {
  INI_ASSERT(ini && key);
  _ini_property_t* property = &ini->properties[0];
  for (int i = 0; i < ini->num_properties; i++, property = &ini->properties[i]) {
    if (property->section == section && strcmp(_ini_str(ini, property->key), key) == 0) {
      return true;
    }
  }
  return false;
}

const char* ini_value(const ini_t* ini, int section, const char* key) {
  INI_ASSERT(ini && key);
  _ini_property_t* property = &ini->properties[0];
  for (int i = 0; i < ini->num_properties; i++, property = &ini->properties[i]) {
    if (property->section == section && strcmp(_ini_str(ini, property->key), key) == 0) {
      return _ini_str(ini, property->value);
    }
  }
  return NULL;
}

int ini_value_as_int(const ini_t* ini, int section, const char* key) {
  return atoi(ini_value(ini, section, key));
}

float ini_value_as_float(const ini_t* ini, int section, const char* key) {
  return atof(ini_value(ini, section, key));
}

bool ini_value_as_bool(const ini_t* ini, int section, const char* key) {
  const char* value = ini_value(ini, section, key);
  return (strcmp(value, "true") == 0);
}

#endif /* INI_IMPLEMENTATION */