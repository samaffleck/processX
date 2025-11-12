file(REMOVE_RECURSE
  "libsundials_sundomeigestpower.a"
  "libsundials_sundomeigestpower.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_sundomeigestpower_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
