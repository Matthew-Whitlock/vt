
#include "transport.h"
#include <cstdlib>

using namespace vt;

static NodeType my_node = uninitialized_destination;
static NodeType num_nodes = uninitialized_destination;

static void fn_test(int a, int b, bool x) {
  printf("fn: a=%d, b=%d, x=%s\n", a, b, x ? "true" : "false");
}

static void fn_test2(int x, int y) {
  printf("fn2: x=%d,y=%d\n",x,y);
}

static void fn_test3(int x, double y) {
  printf("fn3: x=%d,y=%f\n",x,y);
}

struct FunctorTest1 {
  void operator()(int x, double y) const {
    printf("FunctorTest1: x=%d,y=%f\n",x,y);
  }
};

int main(int argc, char** argv) {
  CollectiveOps::initializeContext(argc, argv);
  CollectiveOps::initializeRuntime();

  my_node = theContext->getNode();
  num_nodes = theContext->getNumNodes();

  if (num_nodes == 1) {
    fprintf(stderr, "Please run with at least two ranks!\n");
    fprintf(stderr, "\t mpirun-mpich-clang -n 2 %s\n", argv[0]);
    exit(1);
  }

  if (my_node == 0) {
    theParam->send_data(1, build_data(10, 20, false), param_function_rhs(fn_test));
    theParam->send_data(1, param_function_rhs(fn_test), 50, 29, false);
    theParam->send_data<param_function(fn_test)>(1, build_data(10, 20, false));
    theParam->send_data<param_function(fn_test)>(1, 45, 23, true);

    theParam->send_data<param_function(fn_test2)>(1, 20, 10);
    theParam->send_data<param_function(fn_test3)>(1, 20, 50.0);

    theParam->send_data<FunctorTest1>(1, build_data(20, 50.0));
    theParam->send_data<FunctorTest1>(1, 20, 100.0);
    theParam->send_data<FunctorTest1>(1, build_data(10, 70.0));
  }

  while (1) {
    run_scheduler();
  }

  return 0;
}
