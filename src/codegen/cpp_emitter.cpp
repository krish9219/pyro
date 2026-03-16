#include "codegen/codegen.h"

// Additional C++ emission utilities
// This file provides supplementary code generation features

namespace pyro {

void CodeGenerator::emit_runtime_includes() {
    // Emit platform-specific includes
    #ifdef _WIN32
    emit_line("#define NOVA_WINDOWS 1");
    #elif __APPLE__
    emit_line("#define NOVA_MACOS 1");
    #else
    emit_line("#define NOVA_LINUX 1");
    #endif
}

} // namespace pyro
