#include "soft-fp.h"
#include "single.h"

int FADDS(void *rd, void *rs2, void *rs1)
{
	FP_DECL_S(A); FP_DECL_S(B); FP_DECL_S(R);

	__FP_UNPACK_S(A, rs1);
	__FP_UNPACK_S(B, rs2);
	FP_ADD_S(R, A, B);
	return __FP_PACK_S(rd, R);
}
