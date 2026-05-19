# Tuning `task_granularity_k` for Queue‑Mode Convolution

This document presents benchmark results for selecting the optimal `task_granularity_k`
value. This parameter determines how many chunks each convolution task is split into
when processing images in pipeline mode (i.e., with both `--queue` and `--mode` enabled).

**Definition:**  

`N = task_granularity_k × T`  
- `N` – number of sub‑tasks each convolution task is split into
- `T` – number of OpenMP threads  

This formula defines how a single convolution task is subdivided when processing images in **queue mode** with the `--mode` option enabled.

The value `N` is directly used in the `taskloop` OpemMP directive:

```c
#pragma omp taskloop num_tasks(task_granularity_k * T)
for (int i = 0; i < total_work; i++) {
    // each sub‑task processes a portion of the image
}
```

## Benchmark Results

### For small pipeline size

| Pipeline size (imgs) |  Image size |  Task_granularity_k |  Time (sec)  |
| -------------------- | ----------- | ------------------- | ------------ |
| 4                    |  SMALL      |  1                  |  0.037084    |
| 4                    |  SMALL      |  2                  |  0.035612    |
| 4                    |  SMALL      |  3                  |  0.035927    |
| 4                    |  SMALL      |  4                  |  0.035468    |
| 4                    |  SMALL      |  5                  |  0.034227    |
| 4                    |  SMALL      |  6                  |  0.034913    |
| 4                    |  SMALL      |  7                  |  0.034167    |
| 4                    |  SMALL      |  8                  |  0.033597    |
| 4                    |  SMALL      |  9                  |  0.034275    |
| 4                    |  SMALL      |  10                 |  0.033860    |
| 4                    |  SMALL      |  11                 |  0.035522    |
| 4                    |  SMALL      |  12                 |  0.035755    |
| 4                    |  MIDDLE     |  1                  |  1.424069    |
| 4                    |  MIDDLE     |  2                  |  1.422993    |
| 4                    |  MIDDLE     |  3                  |  1.445473    |
| 4                    |  MIDDLE     |  4                  |  1.456926    |
| 4                    |  MIDDLE     |  5                  |  1.473641    |
| 4                    |  MIDDLE     |  6                  |  1.479498    |
| 4                    |  MIDDLE     |  7                  |  1.495738    |
| 4                    |  MIDDLE     |  8                  |  1.497307    |
| 4                    |  MIDDLE     |  9                  |  1.510926    |
| 4                    |  MIDDLE     |  10                 |  1.508832    |
| 4                    |  MIDDLE     |  11                 |  1.514187    |
| 4                    |  MIDDLE     |  12                 |  1.516234    |
| 4                    |  BIG        |  1                  |  6.254194    |
| 4                    |  BIG        |  2                  |  6.214692    |
| 4                    |  BIG        |  3                  |  6.300145    |
| 4                    |  BIG        |  4                  |  6.310858    |
| 4                    |  BIG        |  5                  |  6.316883    |
| 4                    |  BIG        |  6                  |  6.378516    |
| 4                    |  BIG        |  7                  |  6.332995    |
| 4                    |  BIG        |  8                  |  6.354745    |
| 4                    |  BIG        |  9                  |  6.347267    |
| 4                    |  BIG        |  10                 |  6.349752    |
| 4                    |  BIG        |  11                 |  6.386235    |
| 4                    |  BIG        |  12                 |  6.454156    |
| 4                    |  RANDOM     |  1                  |  4.742078    |
| 4                    |  RANDOM     |  2                  |  4.647056    |
| 4                    |  RANDOM     |  3                  |  4.624216    |
| 4                    |  RANDOM     |  4                  |  4.525026    |
| 4                    |  RANDOM     |  5                  |  4.508677    |
| 4                    |  RANDOM     |  6                  |  4.485636    |
| 4                    |  RANDOM     |  7                  |  4.507360    |
| 4                    |  RANDOM     |  8                  |  4.484135    |
| 4                    |  RANDOM     |  9                  |  4.535308    |
| 4                    |  RANDOM     |  10                 |  4.524583    |
| 4                    |  RANDOM     |  11                 |  4.556986    |
| 4                    |  RANDOM     |  12                 |  4.562545    |

##### Optimal Value:
Based on the benchmark results, the optimal values for different image sizes are as follows:

| Scenario | Recommended `task_granularity_k` |
|----------|--------------------------------|
| Only small images | **8** |
| Only medium/large images | **1** (or **2**) |
| Mixed or unknown sizes | **6** (or **8**) |

**Default safe choice:** `task_granularity_k = 6`  

- Performs well for random images (`4.4856 s`)
- Does not degrade medium/large images significantly
- Works acceptably for small images (close to best)

### For larger pipeline size

| Pipeline size (imgs) |  Image size |  Task_granularity_k |  Time (sec)  |
| -------------------- | ----------- | ------------------- | ------------ |
| 10                   |  SMALL      |  1                  |  0.069875    |
| 10                   |  SMALL      |  2                  |  0.069552    |
| 10                   |  SMALL      |  3                  |  0.069635    |
| 10                   |  SMALL      |  4                  |  0.069426    |
| 10                   |  SMALL      |  5                  |  0.069399    |
| 10                   |  SMALL      |  6                  |  0.068679    |
| 10                   |  SMALL      |  7                  |  0.069695    |
| 10                   |  SMALL      |  8                  |  0.074467    |
| 10                   |  SMALL      |  9                  |  0.076729    |
| 10                   |  SMALL      |  10                 |  0.077053    |
| 10                   |  SMALL      |  11                 |  0.078231    |
| 10                   |  SMALL      |  12                 |  0.082323    |
| 10                   |  MIDDLE     |  1                  |  3.114152    |
| 10                   |  MIDDLE     |  2                  |  3.206073    |
| 10                   |  MIDDLE     |  3                  |  3.268727    |
| 10                   |  MIDDLE     |  4                  |  3.230676    |
| 10                   |  MIDDLE     |  5                  |  3.154166    |
| 10                   |  MIDDLE     |  6                  |  3.270021    |
| 10                   |  MIDDLE     |  7                  |  3.364783    |
| 10                   |  MIDDLE     |  8                  |  3.496756    |
| 10                   |  MIDDLE     |  9                  |  3.584655    |
| 10                   |  MIDDLE     |  10                 |  3.519740    |
| 10                   |  MIDDLE     |  11                 |  3.564722    |
| 10                   |  MIDDLE     |  12                 |  3.524777    |
| 10                   |  BIG        |  1                  |  13.171556   |
| 10                   |  BIG        |  2                  |  13.188246   |
| 10                   |  BIG        |  3                  |  13.117205   |
| 10                   |  BIG        |  4                  |  13.354228   |
| 10                   |  BIG        |  5                  |  13.210490   |
| 10                   |  BIG        |  6                  |  13.400263   |
| 10                   |  BIG        |  7                  |  13.422477   |
| 10                   |  BIG        |  8                  |  14.515064   |
| 10                   |  BIG        |  9                  |  14.871456   |
| 10                   |  BIG        |  10                 |  14.835128   |
| 10                   |  BIG        |  11                 |  14.717948   |
| 10                   |  BIG        |  12                 |  14.789552   |
| 10                   |  RANDOM     |  1                  |  6.238126    |
| 10                   |  RANDOM     |  2                  |  6.295112    |
| 10                   |  RANDOM     |  3                  |  6.325207    |
| 10                   |  RANDOM     |  4                  |  6.262155    |
| 10                   |  RANDOM     |  5                  |  6.415714    |
| 10                   |  RANDOM     |  6                  |  6.266238    |
| 10                   |  RANDOM     |  7                  |  6.198471    |
| 10                   |  RANDOM     |  8                  |  6.189063    |
| 10                   |  RANDOM     |  9                  |  6.206892    |
| 10                   |  RANDOM     |  10                 |  6.087151    |
| 10                   |  RANDOM     |  11                 |  6.124334    |
| 10                   |  RANDOM     |  12                 |  6.172418    |

##### Key Observations

1. **Small images** favour moderate granularity (`k=6`). Unlike the pipeline‑size‑4 case, larger `k` (≥8) causes significant slowdown.
2. **Medium images** prefer the **coarsest granularity** (`k=1`). Any increase in `k` leads to higher execution time.
3. **Large images** show an optimum at `k=3`. The performance drop for `k > 7` is substantial (up to +13%).
4. **Random images** improve with larger `k`, reaching best at `k=10`. This suggests that mixing sizes benefits from finer task decomposition.

##### Optimal Value Selection
Based on the benchmark results, the optimal values for different image sizes are as follows:
| Image type | Recommended `task_granularity_k` |
|------------|--------------------------------|
| **SMALL**  | 6 |
| **MIDDLE** | 1 |
| **BIG**    | 3 |
| **RANDOM** | 10 |


##### Default safe choice 

For an image queue with **unknown or mixed sizes**, `task_granularity_k = 3` is the recommended default:

1. It provides the best result for large images and robust performance across all categories.
2. Values above 8 (especially for small and big images) lead to notable performance penalties.

Depending on the image type, you may choose:

- **If you are sure images are small:** use `k = 6`.
- **If images are medium/large:** stick with `k = 1` or `k = 3` (`3` is safer for mixed medium/large).