# ini.h

a *tiny* library for reading an INI file.

- reads an INI file
- supported platforms: Win32, macOS, Linux
- written in C99

## Usage
**[ini.h](src/ini.h?raw=1)** should be dropped
into an existing project and compiled along with it. The library provides 7 functions for interfacing with a filesystem.

```c
int ini_find_section(const ini_t* ini, const char* name);
bool ini_section_exists(const ini_t* ini, const char* key);
bool ini_property_exists(const ini_t* ini, int section, const char* key);
const char* ini_value(const ini_t* ini, int section, const char* key);
int ini_value_as_int(const ini_t* ini, int section, const char* key);
float ini_value_as_float(const ini_t* ini, int section, const char* key);
bool ini_value_as_bool(const ini_t* ini, int section, const char* key);
```

```c
#include "ini.h"

int main(int argc, char* argv[]) {
  FILE* f = fopen("config.ini", "rb");
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* data = (char*) malloc(size + 1);
  fread(data, 1, size, f);
  data[size] = '\0';
  fclose(f);

  ini_t* ini = ini_load(data);
  free(data);

  int window_id = ini_find_section(ini, "window");
  if (ini_property_exists(ini, window_id, "title")) {
    const char* title = ini_value(ini, window_id, "title")
    /* do something with title */
  }

  ini_destroy(ini);
  return 0;
}
```

See the **[ini.h](src/ini.h?raw=1)** header for a more complete documentation.

## License
This library is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See [LICENSE](LICENSE) for details.