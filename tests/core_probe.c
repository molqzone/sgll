/**
 * @file core_probe.c
 * @brief 核心原语探针：把每个 CSR／时间操作导出为独立符号，供指令级对比。
 *        Core-primitive probe: export each CSR/time operation as its own symbol for instruction-level comparison.
 *
 * @note 仅供 tests/run.py 的 RV64 构建使用；宿主构建不编译本文件。
 *       Used only by the RV64 builds in tests/run.py; host builds do not compile this file.
 * @note 每个函数调用一个公开原语，使自带后端与 SGLL_CORE_BACKEND_SDK 对照后端
 *       可以逐符号比较生成的指令。
 *       Each function calls one public primitive so the built-in backend and the SGLL_CORE_BACKEND_SDK
 * reference backend can be compared symbol by symbol.
 */
#include "inc/sg200x_ll_csr.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void sgll_test_core_mstatus_read(void) { (void)sgll_csr_mstatus_read(); }

    void sgll_test_core_mstatus_write(unsigned long value) { sgll_csr_mstatus_write(value); }

    void sgll_test_core_mie_read(void) { (void)sgll_csr_mie_read(); }

    void sgll_test_core_mie_write(unsigned long value) { sgll_csr_mie_write(value); }

    void sgll_test_core_mie_set(unsigned long mask) { sgll_csr_mie_set(mask); }

    void sgll_test_core_mie_clear(unsigned long mask) { sgll_csr_mie_clear(mask); }

    void sgll_test_core_irq_enable(void) { sgll_csr_global_irq_enable(); }

    void sgll_test_core_irq_disable(void) { sgll_csr_global_irq_disable(); }

    uint64_t sgll_test_core_time_read(void) { return sgll_csr_time_read(); }

#ifdef __cplusplus
}
#endif
