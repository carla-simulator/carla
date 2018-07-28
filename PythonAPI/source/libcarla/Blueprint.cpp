// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/BlueprintLibrary.h>
#include <carla/client/ActorBlueprint.h>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ostream>

template <typename Iterable>
static std::ostream &PrintList(std::ostream &out, const Iterable &list) {
  auto it = list.begin();
  out << '[' << *it;
  for (++it; it != list.end(); ++it) {
    out << ", " << *it;
  }
  out << ']';
  return out;
}

namespace std {

  std::ostream &operator<<(std::ostream &out, const std::vector<std::string> &vector_of_strings) {
    return PrintList(out, vector_of_strings);
  }

} // namespace std

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const Color &color) {
    out << "Color(r=" << color.r
        << ", g=" << color.g
        << ", b=" << color.b << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const ActorBlueprint &bp) {
    out << "ActorBlueprint(id=" << bp.GetId() << "tags=" << bp.GetTags() << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const BlueprintLibrary &blueprints) {
    return PrintList(out, blueprints);
  }

} // namespace client
} // namespace carla

void export_blueprint() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace crpc = carla::rpc;

  enum_<crpc::ActorAttributeType>("ActorAttributeType")
    .value("Bool", crpc::ActorAttributeType::Bool)
    .value("Int", crpc::ActorAttributeType::Int)
    .value("Float", crpc::ActorAttributeType::Float)
    .value("String", crpc::ActorAttributeType::String)
    .value("RGBColor", crpc::ActorAttributeType::RGBColor)
  ;

  class_<cc::Color>("Color")
    .def(init<uint8_t, uint8_t, uint8_t>((arg("r")=0, arg("g")=0, arg("b")=0)))
    .def_readwrite("r", &cc::Color::r)
    .def_readwrite("g", &cc::Color::g)
    .def_readwrite("b", &cc::Color::b)
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<std::string>>("vector_of_strings")
    .def(vector_indexing_suite<std::vector<std::string>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::ActorAttribute>("ActorAttribute", no_init)
    .add_property("is_modifiable", &cc::ActorAttribute::IsModifiable)
    .add_property("type", &cc::ActorAttribute::GetType)
    .add_property("recommended_values", +[](const cc::ActorAttribute &self) -> std::vector<std::string> {
      return self.GetRecommendedValues();
    })
    .def("as_bool", &cc::ActorAttribute::As<bool>)
    .def("as_int", &cc::ActorAttribute::As<int>)
    .def("as_float", &cc::ActorAttribute::As<float>)
    .def("as_str", &cc::ActorAttribute::As<std::string>)
    .def("as_color", &cc::ActorAttribute::As<cc::Color>)
    .def("__eq__", &cc::ActorAttribute::operator==<bool>)
    .def("__eq__", &cc::ActorAttribute::operator==<int>)
    .def("__eq__", &cc::ActorAttribute::operator==<float>)
    .def("__eq__", &cc::ActorAttribute::operator==<std::string>)
    .def("__eq__", &cc::ActorAttribute::operator==<cc::Color>)
    .def("__eq__", &cc::ActorAttribute::operator==<cc::ActorAttribute>)
    .def("__ne__", &cc::ActorAttribute::operator!=<bool>)
    .def("__ne__", &cc::ActorAttribute::operator!=<int>)
    .def("__ne__", &cc::ActorAttribute::operator!=<float>)
    .def("__ne__", &cc::ActorAttribute::operator!=<std::string>)
    .def("__ne__", &cc::ActorAttribute::operator!=<cc::Color>)
    .def("__ne__", &cc::ActorAttribute::operator!=<cc::ActorAttribute>)
    .def("__nonzero__", &cc::ActorAttribute::As<bool>)
    .def("__bool__", &cc::ActorAttribute::As<bool>)
    .def("__int__", &cc::ActorAttribute::As<int>)
    .def("__float__", &cc::ActorAttribute::As<float>)
    .def("__str__", &cc::ActorAttribute::As<std::string>)
  ;

  class_<cc::ActorBlueprint>("ActorBlueprint", no_init)
    .add_property("id", +[](const cc::ActorBlueprint &self) -> std::string {
      return self.GetId();
    })
    .add_property("tags", &cc::ActorBlueprint::GetTags)
    .def("contains_tag", &cc::ActorBlueprint::ContainsTag)
    .def("match_tags", &cc::ActorBlueprint::MatchTags)
    .def("contains_attribute", &cc::ActorBlueprint::ContainsAttribute)
    .def("get_attribute", +[](const cc::ActorBlueprint &self, const std::string &id) -> cc::ActorAttribute {
      return self.GetAttribute(id);
    })
    .def("set_attribute", &cc::ActorBlueprint::SetAttribute)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::BlueprintLibrary, boost::noncopyable, boost::shared_ptr<cc::BlueprintLibrary>>("BlueprintLibrary", no_init)
    .def("find", +[](const cc::BlueprintLibrary &self, const std::string &key) -> cc::ActorBlueprint {
      return self.at(key);
    })
    .def("filter", &cc::BlueprintLibrary::Filter)
    .def("__getitem__", +[](const cc::BlueprintLibrary &self, size_t pos) -> cc::ActorBlueprint {
      return self.at(pos);
    })
    .def("__len__", &cc::BlueprintLibrary::size)
    .def("__iter__", range(&cc::BlueprintLibrary::begin, &cc::BlueprintLibrary::end))
    .def(self_ns::str(self_ns::self))
  ;
}
