# Locate and configure LibTorch / PyTorch C++ libraries across Conan and
# bare-to-metal system installations (such as Fedora/Nobara python3-torch).

if(NOT Torch_DIR)
    execute_process(
        COMMAND python3 -c "import torch; print(torch.utils.cmake_prefix_path, end='')"
        OUTPUT_VARIABLE _TORCH_CMAKE_PREFIX
        RESULT_VARIABLE _TORCH_PY_RESULT
        ERROR_QUIET
    )
    if(_TORCH_PY_RESULT EQUAL 0 AND _TORCH_CMAKE_PREFIX)
        list(APPEND CMAKE_PREFIX_PATH "${_TORCH_CMAKE_PREFIX}")
    endif()
endif()

# System Caffe2Config.cmake (e.g. Fedora/Nobara python3-torch) requires
# protobuf::libprotobuf to exist in the CMake target graph even though
# libtorch_cpu.so links libprotobuf dynamically.
if(NOT TARGET protobuf::libprotobuf)
    add_library(pkai_protobuf_dummy INTERFACE)
    add_library(protobuf::libprotobuf ALIAS pkai_protobuf_dummy)
endif()

find_package(Torch REQUIRED)

# When python3-torch is compiled with ROCm/HIP enabled on Fedora/Nobara,
# Caffe2Targets.cmake attaches torch_hip_library (and hip::amdhip64) to the
# `torch` target even on CPU builds where LoadHIP.cmake sets PYTORCH_FOUND_HIP=FALSE.
# Redirecting `torch` to `torch_cpu` avoids missing HIP CMake target errors and
# prevents loading libtorch_hip.so / libhsa-runtime64.so at runtime.
if(TARGET torch AND TARGET torch_cpu AND NOT PYTORCH_FOUND_HIP)
    get_target_property(_TORCH_CPU_LOC torch_cpu IMPORTED_LOCATION_RELWITHDEBINFO)
    if(_TORCH_CPU_LOC)
        set_target_properties(torch PROPERTIES
            INTERFACE_LINK_LIBRARIES "torch_cpu_library"
            IMPORTED_LOCATION_RELWITHDEBINFO "${_TORCH_CPU_LOC}"
            IMPORTED_SONAME_RELWITHDEBINFO "libtorch_cpu.so"
        )
    endif()
endif()
