#include<stdarg.h>
#include<stddef.h>
#include<setjmp.h>
#include<string.h>
#include<cmocka.h>
#include<qthread/performance.h>
#include<qthread/logging.h>

typedef enum {
  S1,S2,S3,S4,S5,NumStates1
} state1_t;

const char* state1_names[] = {
    "S1","S2","S3","S4","S5"
};

qtstategroup_t* group1=NULL;
qtstategroup_t* group2=NULL;
qtstategroup_t* group3=NULL;

static void teardown(void** state){
  qtperf_free_data();
  group1 = NULL;
  group2 = NULL;
  group3 = NULL;
}

static void test_create_group(void** state){
  group1 = qtperf_create_state_group(NumStates1, state1_names);
  assert_true(group1 != NULL);
  assert_true(qtperf_check_invariants());
}

static void test_get_perfdata(void**state) {
  qtperfdata_t* data = NULL;
  assert_true(group1 != NULL);
  data = qtperf_create_perfdata(group1);
  assert_true(data != NULL);
  assert_true(qtperf_check_invariants());
  qtperf_free_data();
  assert_true(qtperf_check_invariants() && "after cleanup");
}

static void test_check_invariants(void** state) {
  assert_true(qtperf_check_invariants());
}

int main(int argc, char** argv){
  const struct CMUnitTest test[] ={
    cmocka_unit_test(test_check_invariants),
    cmocka_unit_test(test_create_group),
    cmocka_unit_test(test_get_perfdata)
  };
  return cmocka_run_group_tests(test,NULL,NULL);
}