# The pinned debugserver headers use shared_ptr and enable_shared_from_this
# without including <memory>. Supply the header only in this subproject's
# C++ compilations; leave C, assembly and other LLVM targets unchanged.
add_compile_options("$<$<COMPILE_LANGUAGE:CXX,OBJCXX>:SHELL:-include memory>")
