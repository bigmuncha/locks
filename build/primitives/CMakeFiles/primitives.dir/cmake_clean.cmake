file(REMOVE_RECURSE
  "libprimitives.a"
  "libprimitives.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/primitives.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
