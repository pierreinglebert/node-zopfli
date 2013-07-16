{
  "targets": [
    {
      "target_name": "zopflipng",
      "include_dirs": [
        "zopfli/src/zopflipng"
      ],
      "sources": [ 
      	"src/zopflipng.cc",
      	"zopfli/src/zopfli/blocksplitter.c",
      	"zopfli/src/zopfli/cache.c",
      	"zopfli/src/zopfli/deflate.c",
      	"zopfli/src/zopfli/hash.c",
      	"zopfli/src/zopfli/katajainen.c",
      	"zopfli/src/zopfli/lz77.c",
      	"zopfli/src/zopfli/squeeze.c",
      	"zopfli/src/zopfli/tree.c",
      	"zopfli/src/zopfli/util.c",
      	"zopfli/src/zopfli/zopfli_lib.c",
      	"zopfli/src/zopflipng/zopflipng_lib.cc",
      	"zopfli/src/zopflipng/lodepng/lodepng.cpp",
      	"zopfli/src/zopflipng/lodepng/lodepng_util.cpp"
      ]
    }
  ]
}