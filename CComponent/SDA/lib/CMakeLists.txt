
# Due to a mutual dependencies, all the sub projects of the SVG are merged here
# Otherwise it is impossible to load the dependencies in opt
# NOTE: if the ldi should be linked into opt, we should probably use the individual sub projects here, rather than the combined project 
set(SOURCES
    common/VisitDir.cpp
	common/MultiTask.cpp
	common/SoftPara.cpp
	common/Stat.cpp	
	ModuleSet.cpp
	ExternalLib.cpp
    )

#add_llvm_loadable_module(sda ${SOURCES})
add_llvm_Library(LLVMsda ${SOURCES})

llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support )
target_link_libraries(LLVMsda ${llvm_libs})

#target_link_libraries(sda ${llvm_libs})

if(DEFINED IN_SOURCE_BUILD)
    #add_dependencies(sda intrinsics_gen)
    add_dependencies(LLVMsda intrinsics_gen)
endif()
