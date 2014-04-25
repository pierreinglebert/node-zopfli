{
  'target_defaults': {
    'default_configuration': 'Release',
    'configurations': {
      'Debug': {
        'cflags': ['-g3', '-O0'],
        'msvs_settings': {
          'VCLinkerTool': {
            'GenerateDebugInformation': 'true',
            'LinkIncremental': 2
          }
        }
      },
      'Release': {
        'cflags': ['-O2', '-W', '-Wall', '-Wextra', '-ansi', '-pedantic'],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'Optimization': 3, # -O3
            'FavorSizeOrSpeed': 1, # favor speed
          },
          'VCLinkerTool': {
            'OptimizeReferences': 2, # /OPT:REF
          }
        }
      },
    },
  },
  "targets": [
    {
      "target_name": "<(module_name)",
      'lflags': ['-lm'],
      "include_dirs": [
        "zopfli/src/zopfli",
        "zopfli/src/zopflipng",
        "<!(node -e \"require('nan')\")"
      ],
      "sources": [
        "src/zopfli-binding.cc",
      	"src/png/zopflipng.cc",
      	"zopfli/src/zopfli/blocksplitter.c",
      	"zopfli/src/zopfli/cache.c",
      	"zopfli/src/zopfli/deflate.c",
        "zopfli/src/zopfli/gzip_container.c",
      	"zopfli/src/zopfli/hash.c",
      	"zopfli/src/zopfli/katajainen.c",
      	"zopfli/src/zopfli/lz77.c",
      	"zopfli/src/zopfli/squeeze.c",
      	"zopfli/src/zopfli/tree.c",
      	"zopfli/src/zopfli/util.c",
        "zopfli/src/zopfli/zlib_container.c",
      	"zopfli/src/zopfli/zopfli_lib.c",
      	"zopfli/src/zopflipng/zopflipng_lib.cc",
      	"zopfli/src/zopflipng/lodepng/lodepng.cpp",
      	"zopfli/src/zopflipng/lodepng/lodepng_util.cpp"
      ],
      "cflags": [
          "-Wall",
          "-O3"
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
          {
            "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
            "destination": "<(module_path)"
          }
      ]
    }
  ]
}
