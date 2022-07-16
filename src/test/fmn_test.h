#ifndef FMN_TEST_H
#define FMN_TEST_H

#include <stdio.h>
#include <string.h>

#define FMN_ITEST(name,...) int name()
#define XXX_FMN_ITEST(name,...) int name()

#define FMN_UTEST(name,...) if (fmn_test_filter(#name,argc,argv,0,#__VA_ARGS__)) { \
  fmn_report_test_result(name(),#name); \
} else fmn_report_test_skipped(#name);
#define XXX_FMN_UTEST(name,...) if (fmn_test_filter(#name,argc,argv,1,#__VA_ARGS__)) { \
  fmn_report_test_result(name(),#name); \
} else fmn_report_test_skipped(#name);

int fmn_test_filter(const char *name,int argc,char **argv,int ignore,const char *tags);
void fmn_report_test_result(int result,const char *name);
void fmn_report_test_skipped(const char *name);

/* Logging.
 ******************************************************************/
 
#define FMN_FAILURE_RESULT -1
 
#define FMN_FAIL_MORE(k,fmt,...) fprintf(stderr,"FMN_TEST DETAIL | %20s: "fmt"\n",k,##__VA_ARGS__);

#define FMN_FAIL_BEGIN(fmt,...) { \
  fprintf(stderr,"FMN_TEST DETAIL +--------------------------------------------------\n"); \
  fprintf(stderr,"FMN_TEST DETAIL | ASSERTION FAILED\n"); \
  fprintf(stderr,"FMN_TEST DETAIL +--------------------------------------------------\n"); \
  if (fmt&&fmt[0]) FMN_FAIL_MORE("Message",fmt,##__VA_ARGS__) \
  FMN_FAIL_MORE("Location","%s:%d",__FILE__,__LINE__) \
}

#define FMN_FAIL_END { \
  fprintf(stderr,"FMN_TEST DETAIL +--------------------------------------------------\n"); \
  return FMN_FAILURE_RESULT; \
}

/* Assertions.
 *******************************************************************/
 
#define FMN_ASSERT(condition,...) if (!(condition)) { \
  FMN_FAIL_BEGIN(""__VA_ARGS__) \
  FMN_FAIL_MORE("Expected","true") \
  FMN_FAIL_MORE("As written","%s",#condition) \
  FMN_FAIL_END \
}

#define FMN_ASSERT_NOT(condition,...) if (condition) { \
  FMN_FAIL_BEGIN(""__VA_ARGS__) \
  FMN_FAI_MORE("Expected","false") \
  FMN_FAIL_MORE("As written","%s",#condition) \
  FMN_FAIL_END \
}

#define FMN_ASSERT_CALL(call,...) { \
  int _result=(int)(call); \
  if (_result<0) { \
    FMN_FAIL_BEGIN(""__VA_ARGS__) \
    FMN_FAIL_MORE("Expected","successful call") \
    FMN_FAIL_MORE("Result","%d",_result) \
    FMN_FAIL_MORE("As written","%s",#call) \
    FMN_FAIL_END \
  } \
}

#define FMN_ASSERT_FAILURE(call,...) { \
  int_result=(int)(call); \
  if (_result>=0) { \
    FMN_FAIL_BEGIN(""__VA_ARGS__) \
    FMN_FAIL_MORE("Expected","failed call") \
    FMN_FAIL_MORE("Result","%d",_result) \
    FMN_FAIL_MORE("As written","%s",#call) \
    FMN_FAIL_END \
  } \
}

#define FMN_ASSERT_INTS_OP(a,op,b,...) { \
  int _a=(int)(a),_b=(int)(b); \
  if (!(_a op _b)) { \
    FMN_FAIL_BEGIN(""__VA_ARGS__) \
    FMN_FAIL_MORE("As written","%s %s %s",#a,#op,#b) \
    FMN_FAIL_MORE("Values","%d %s %d",_a,#op,_b) \
    FMN_FAIL_END \
  } \
}

#define FMN_ASSERT_INTS(a,b,...) FMN_ASSERT_INTS_OP(a,==,b,##__VA_ARGS__)

//TODO string assertions

#endif
