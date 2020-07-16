# If we don't need RTTI or EH, there's no reason to export anything
# from the hello plugin.
if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/IBSpass.exports)
  endif()
endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

add_llvm_library( llvmIBSpass MODULE BUILDTREE_ONLY
  IBSpass.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/IBSpass

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
