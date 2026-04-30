# Conv
Apply convolution filters (blur, sharpen, edge detection, etc.) to images
## Usage
```
conv --input=<input_file> --filter=<filter> --size=<size> --mode=<mode> [--clean]
```
_The utility has no required arguments. All settings are optional and have default values._

#### Options
| Option | Default | Possible values |
|--------|---------|-----------------|
| `--input` | First image in `./images` directory | Name of image file inside `./images` (e.g., `photo.jpg`) |
| `--filter` | `blur` | `blur`, `sharpen`, `edge`, `emboss`, `motion` |
| `--size` | `3` | Any odd number from 3 to 13 |
| `--mode` | `seq` | `seq`, `pixel`, `row`, `column`, `block` |

#### Flags

| Flag | Description |
|------|-------------|
| `--clean`, `-c` | Remove all files from the `./output` directory before writing new results |
| `--help`, `-h` | Print help information with all available options and flags |

## Quick Start

### Build and run conv

```bash
cd ./build
cmake ..
make build
./conv
```
