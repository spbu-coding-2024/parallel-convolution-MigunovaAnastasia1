# Conv
Apply convolution filters (blur, sharpen, edge detection, etc.) to images
## Usage
```
conv --input=<input_file> --filter=<filter> --size=<size> --mode=<mode> [--clean]
```
_The utility has no required arguments. All settings are optional and have default values._

#### Options
| Option     | Default                             | Possible values                                          |
| ---------- | ----------------------------------- | -------------------------------------------------------- |
| `--input`  | First image in `./images` directory | Name of image file inside `./images` (e.g., `photo.jpg`) |
| `--filter` | `blur`                              | `blur`, `sharpen`, `edge`, `emboss`, `motion`            |
| `--size`   | `3`                                 | Any odd number from 3 to 13                              |
| `--mode`   | `seq`                               | `seq`, `pixel`, `row`, `column`, `block`                 |

#### Flags

| Flag            | Description                                                               |
| --------------- | ------------------------------------------------------------------------- |
| `--clean`, `-c` | Remove all files from the `./output` directory before writing new results |
| `--help`, `-h`  | Print help information with all available options and flags               |

## Quick Start

Before running conv, benchmarks or tests, create an `images` directory in `build` directory and add at least one image file (`.png`, `.jpg`, `.jpeg`, or `.bmp`).

### Build and run conv

```bash
mkdir build
cd ./build
cmake ..
make build
./conv
```
### Run benchmarks

#### Run benchmarks with default parameters

```bash
# inside ./build after cmake ..
make bench
```

#### Run benchmarks with custom parameters

You can override default parameters using CMake variables:

| Variable            | Description                                                             | Default                             |
| ------------------- | ----------------------------------------------------------------------- | ----------------------------------- |
| `-DMY_BENCH_SIZE`   | Kernel size (odd number from 3 to 13)                                   | `3`                                 |
| `-DMY_BENCH_FILTER` | Filter type (`blur`, `sharpen`, `edge`, `emboss`, `gaussian`, `motion`) | `blur`                              |
| `-DMY_BENCH_INPUT`  | Input image name (must be in `./images/` directory)                     | first image in `./images` directory |

> Note: All MY_BENCH_* variables are optional. Only pass the ones you need to change.

Usage:

```bash
# inside ./build
cmake -DMY_BENCH_SIZE=<kernel_size> -DMY_BENCH_FILTER=<filter> -DMY_BENCH_INPUT=<name_of_input_image> ..
make bench
```

Example:

```bash
# inside ./build
cmake -DMY_BENCH_SIZE=7 -DMY_BENCH_INPUT=300x120.jpg ..
make bench
```

### Run tests
```bash
# inside ./build after cmake ..
make test
```
## Benchmarks
### Cache configuration
<p align="center">
  <img src="benchmark/cache-conf.png" alt="Cache-conf" width="800" style="border: 1px solid #ddd; border-radius: 4px;">
  <br>
  <em>The benchmarks were conducted on the following system:</em>
</p>

###### **12 logical cores - the ability to run 12 threads in parallel

### Benchmark results

#### Bench 1
<p align="center">
  <img src="benchmark/results1.png" alt="Bench 1" width="800" style="border: 1px solid #ddd; border-radius: 4px;">
  <br>
  <em>Small image</em>
</p>

#### Bench 2
<p align="center">
  <img src="benchmark/results2.png" alt="Bench 2" width="800" style="border: 1px solid #ddd; border-radius: 4px;">
  <br>
  <em>Medium image</em>
</p>

#### Bench 3
<p align="center">
  <img src="benchmark/results3.png" alt="Bench 3" width="800" style="border: 1px solid #ddd; border-radius: 4px;">
  <br>
  <em>Big image</em>
</p>


## Analysis

#### Row mode

* Row‑based parallelisation makes very good use of spatial cache locality, which results in few cache misses. It consistently delivers among the best performance across all image sizes.

#### Pixel mode

* Pixel‑based parallelisation strategy yields almost identical performance to the row‑based strategy. Both approaches traverse the image in row‑major order, preserving spatial locality and sequential memory access. The only difference is that in the row‑based strategy, threads are assigned whole rows, whereas in the pixel‑based strategy, threads receive whole rows plus possibly a partial row at the edges.

#### Column mode

* For relatively small images (those that fit into the cache of the test hardware), column mode performs comparably to row and block modes. However, for large images that exceed cache capacity, its performance degrades significantly — becoming more than twice as slow as other parallel modes.

#### Block mode

* Block mode demonstrates solid performance on images of any size, achieving results close to those of row and pixel modes, though slightly below them. It also exhibits good spatial cache locality, making it a reliable choice across different workloads