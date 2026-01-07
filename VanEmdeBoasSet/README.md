# VanEmdeBoasTree

This implementation provides a `std::set`-compatible interface for integer types, optimized for high performance and memory efficiency. 
The architecture decomposes the classic Van Emde Boas tree into two distinct layers to minimize slow memory allocations overhead in low tree levels:

1. **Internal Level:** Follows the classic recursive vEB structure.
2. **Basic Level:** Implements a flat, allocation-free bitmask structure for the leaf nodes.

## Performance comparison with std::set

|     ![plot](./benchmark_Insert.png)                    |        ![plot](./benchmark_Insert_log_scale.png)                   |
|:------------------------------------------------------:|:------------------------------------------------------------------:|



|     ![plot](./benchmark_Contains.png)                    |        ![plot](./benchmark_Contains_log_scale.png)                   |
|:------------------------------------------------------:|:------------------------------------------------------------------:|



|     ![plot](./benchmark_Next.png)                    |        ![plot](./benchmark_Next_log_scale.png)                   |
|:------------------------------------------------------:|:------------------------------------------------------------------:|

*note that on small number of elements structure works worse than std::set
