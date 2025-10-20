// // src/plant.cpp
// #pragma once

// #include <vector>
// #include <unordered_map>
// #include <stdexcept>

// #include "processX/unitop.h"
// #include "processX/stream.h"

// namespace px {

//   template <typename T, typename type_enum, typename Factory>
//   class PlantObject {
//   public:
//     std::vector<T> objects;
//     std::unordered_map<Handle, std::size_t> obj_idx;
//     IdGen obj_ids{};
//     Factory factory{};

//     Handle AddObj(type_enum type) {
//       Handle id = obj_ids.make();
//       objects.emplace_back(factory(id, type));
//       obj_idx[id] = objects.size() - 1;
//       return id;
//     }

//     bool RemoveObj(Handle h) {
//       auto it = obj_idx.find(h);
//       if (it == obj_idx.end()) return false;

//       std::size_t idx = it->second;
//       std::size_t last = objects.size() - 1;

//       if (idx != last) {
//         std::swap(objects[idx], objects[last]);
//         obj_idx[objects[idx].id] = idx;
//       }

//       objects.pop_back();
//       obj_idx.erase(it);
//       return true;
//     }

//     T& GetObj(Handle h) {
//       return objects.at(obj_idx.at(h));
//     }
//     const T& GetObj(Handle h) const {
//       return objects.at(obj_idx.at(h));
//     }

//     T* TryGetObj(Handle h) {
//       auto it = obj_idx.find(h);
//       return it == obj_idx.end() ? nullptr : &objects[it->second];
//     }
//     const T* TryGetObj(Handle h) const {
//       auto it = obj_idx.find(h);
//       return it == obj_idx.end() ? nullptr : &objects[it->second];
//     }

//     bool Contains(Handle h) const {
//       return obj_idx.find(h) != obj_idx.end();
//     }
//   };

//   class Plant {
//   public:
//     PlantObject<UnitOp, UnitType, MakeUnitFn> units;

//     void Connect();
//   };

// } // namespace px
