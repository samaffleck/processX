file(REMOVE_RECURSE
  "libsundials_sunlinsoldense.a"
  "libsundials_sunlinsoldense.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_sunlinsoldense_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
