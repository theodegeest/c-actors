#include <criterion/criterion.h>

#include "../src/file.h"

Test(project_test, example) {
  cr_expect(1 == 1, "This is an example.");
}

Test(project_test, example_2) {
  cr_expect(doStuff() == 0, "This is also an example.");
}
