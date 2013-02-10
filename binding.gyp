{
    "targets": [
        {
            "target_name"  : "texture_compiler",
            "defines"      : [
                "__STDC_LIMIT_MACROS",
                "__STDC_CONSTANT_MACROS",
                "BUILDING_NODE_EXTENSION",
                "STBI_SIMD"
            ],
            "include_dirs" : [
                "src"
            ],
            "sources"      : [
                "src/libimage.cpp",
                "src/compiler.cpp",
                "src/v8module.cpp"
            ],
            "conditions"   : [
            ]
        }
    ]
}
