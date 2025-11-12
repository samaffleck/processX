file(REMOVE_RECURSE
  "libsundials_idas.a"
  "libsundials_idas.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_idas_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
