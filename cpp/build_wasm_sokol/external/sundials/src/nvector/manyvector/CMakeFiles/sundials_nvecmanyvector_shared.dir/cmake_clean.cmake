file(REMOVE_RECURSE
  "libsundials_nvecmanyvector.a"
  "libsundials_nvecmanyvector.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_nvecmanyvector_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
