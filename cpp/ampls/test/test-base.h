#ifndef _TEST_BASE_H_
#define _TEST_BASE_H_

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <gtest/gtest.h>

#define ampls_test(NAME, MODELCLASS)  \
  TEST(NAME, MODELCLASS) { example<ampls::MODELCLASS>();}

#endif