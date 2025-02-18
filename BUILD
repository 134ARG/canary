config_setting(
    name = "dbg",
    values = {"compilation_mode": "dbg"},
)

cc_library(
    name = "support",
    srcs = glob([
        "lib/Support/*.cpp",
    ]),
    hdrs = glob([
        "include/Support/*.h",
    ]),
    defines = [
        "LLVM14",
    ],
    includes = ["include"],
    deps = [
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "canary-dyck-aa",
    srcs = glob([
        "lib/DyckAA/*.cpp",
        "lib/DyckAA/*.h",
    ]),
    hdrs = glob([
        "include/DyckAA/*.h",
    ]),
    copts = select(
        {
            ":dbg": ["-pg"],
            "//conditions:default": [],
        },
    ),
    defines = [
        "LLVM14",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        ":support",
        "@llvm-project//llvm:Analysis",
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "canary-transform",
    srcs = glob([
        "lib/Transform/*.cpp",
    ]),
    hdrs = glob([
        "include/Transform/*.h",
    ]),
    defines = [
        "LLVM14",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        ":support",
        "@llvm-project//llvm:Core",
    ],
)

cc_library(
    name = "canary-nullpointer",
    srcs = glob([
        "lib/NullPointer/*.cpp",
    ]),
    hdrs = glob([
        "include/NullPointer/*.h",
    ]),
    defines = [
        "LLVM14",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        ":canary-dyck-aa",
        ":support",
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:Support",
    ],
)
