#define ASSERT(v1, v2) \
  {\
    if ((v1) != (v2)) {				\
      printf("Assertion failed %d != %d\n", (int)(v1), (int)(v2));	\
      exit(1);\
    }}	      
