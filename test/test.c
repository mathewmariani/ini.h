#include "acutest.h"

#define INI_IMPL
#include "ini.h"

/* helper functions */

ini_t* ini_load_from_file(const char* filename) {
  FILE* f = fopen(filename, "rb");
  if (!f) {
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* data = (char*) malloc(size + 1);
  fread(data, 1, size, f);
  data[size] = '\0';
  fclose(f);

  ini_t* ini = ini_load(data);
  free(data);
  return ini;
}

/* internal functions */

void test_ini_find_section(void) {
  ini_t* ini = ini_load_from_file("test.ini");

  TEST_CHECK(ini_find_section(ini, "owner") == 1);
  TEST_CHECK(ini_find_section(ini, "database") == 2);
  TEST_CHECK(ini_find_section(ini, "nope") == -1);
}

void test_ini_section_exists(void) {
  ini_t* ini = ini_load_from_file("test.ini");

  TEST_CHECK(ini_section_exists(ini, "owner") == true);
  TEST_CHECK(ini_section_exists(ini, "database") == true);
  TEST_CHECK(ini_section_exists(ini, "nope") == false);
}

void test_ini_property_exists(void) {
  ini_t* ini = ini_load_from_file("test.ini");

  int owner_id = ini_find_section(ini, "owner");
  int database_id = ini_find_section(ini, "database");

  TEST_CHECK(ini_property_exists(ini, INI_GLOBAL_SECTION, "network") == true);

  TEST_CHECK(ini_property_exists(ini, owner_id, "name") == true);
  TEST_CHECK(ini_property_exists(ini, owner_id, "organization") == true);
  TEST_CHECK(ini_property_exists(ini, owner_id, "nope") == false);

  TEST_CHECK(ini_property_exists(ini, database_id, "server") == true);
  TEST_CHECK(ini_property_exists(ini, database_id, "port") == true);
  TEST_CHECK(ini_property_exists(ini, database_id, "file") == true);
  TEST_CHECK(ini_property_exists(ini, database_id, "nope") == false);
}

void test_ini_value(void) {
  ini_t* ini = ini_load_from_file("test.ini");

  int owner_id = ini_find_section(ini, "owner");
  int database_id = ini_find_section(ini, "database");

  TEST_CHECK(strcmp(ini_value(ini, INI_GLOBAL_SECTION, "network"), "wireless") == 0);

  TEST_CHECK(strcmp(ini_value(ini, owner_id, "name"), "John Doe") == 0);
  TEST_CHECK(strcmp(ini_value(ini, owner_id, "organization"), "Acme Widgets Inc.") == 0);
  TEST_CHECK(ini_value(ini, owner_id, "nope") == NULL);

  TEST_CHECK(strcmp(ini_value(ini, database_id, "server"), "192.0.2.62") == 0);
  TEST_CHECK(strcmp(ini_value(ini, database_id, "port"), "143") == 0);
  TEST_CHECK(strcmp(ini_value(ini, database_id, "file"), "payroll.dat") == 0);
  TEST_CHECK(ini_value(ini, database_id, "nope") == NULL);

  TEST_CHECK(strcmp(ini_value(ini, database_id, "server"), "192.0.2.62") == 0);
  TEST_CHECK(strcmp(ini_value(ini, database_id, "port"), "143") == 0);
  TEST_CHECK(strcmp(ini_value(ini, database_id, "file"), "payroll.dat") == 0);
  TEST_CHECK(ini_value(ini, database_id, "nope") == NULL);
}

void test_ini_value_as_int(void) {
  ini_t* ini = ini_load_from_file("test.ini");

  int variables_id = ini_find_section(ini, "variables");

  TEST_CHECK(ini_value_as_int(ini, variables_id, "int") == 1234);
  TEST_CHECK(ini_value_as_int(ini, variables_id, "float") == 12);
}

void test_ini_value_as_float(void) {
  ini_t* ini = ini_load_from_file("test.ini");

  int variables_id = ini_find_section(ini, "variables");

  TEST_CHECK(ini_value_as_float(ini, variables_id, "int") == 1234.0f);
  TEST_CHECK(ini_value_as_float(ini, variables_id, "float") == 12.34f);
}

void test_ini_value_as_bool(void) {
  ini_t* ini = ini_load_from_file("test.ini");

  int variables_id = ini_find_section(ini, "variables");

  TEST_CHECK(ini_value_as_bool(ini, variables_id, "int") == false);
  TEST_CHECK(ini_value_as_bool(ini, variables_id, "float") == false);
  TEST_CHECK(ini_value_as_bool(ini, variables_id, "bool") == true);
  TEST_CHECK(ini_value_as_bool(ini, variables_id, "string") == false);
}

TEST_LIST = {
  /* internal functions */
  { "ini_find_section", test_ini_find_section },
  { "ini_section_exists", test_ini_section_exists },
  { "ini_property_exists", test_ini_property_exists },
  { "ini_value", test_ini_value },
  { "ini_value_as_int", test_ini_value_as_int },
  { "ini_value_as_float", test_ini_value_as_float },
  { "ini_value_as_bool", test_ini_value_as_bool },

  /* always last. */
  { NULL, NULL }
};