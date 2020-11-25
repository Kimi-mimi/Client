//
// Created by Dmitry Gorin on 25.11.2020.
//

#include <CUnit/Basic.h>
#include "bytes/test/string_test.h"

int main(void) {
    int ret;
    CU_pSuite stringSuite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS) {
        printf("Can't initialize cu_registry\n");
        return CU_get_error();
    }

    stringSuite = CU_add_suite("String module tests", initSuiteString, cleanupSuiteString);
    if (!stringSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((ret = fillSuiteWithTestsString(stringSuite)) != CUE_SUCCESS) {
        CU_cleanup_registry();
        return ret;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    printf("Exiting tests\n");
    return CU_get_error();
}
