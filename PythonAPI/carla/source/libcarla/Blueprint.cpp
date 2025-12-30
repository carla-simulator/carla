// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/actors/BlueprintLibrary.h>
#include <carla/actors/ActorBlueprint.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ostream>

namespace carla {

namespace sensor {
namespace data {

  std::ostream &operator<<(std::ostream &out, const Color &color) {
    out << "Color(" << std::to_string(color.r)
        << ',' << std::to_string(color.g)
        << ',' << std::to_string(color.b)
        << ',' << std::to_string(color.a) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const OpticalFlowPixel &color) {
    out << "Color(" << std::to_string(color.x)
        << ',' << std::to_string(color.y) << ')';
    return out;
  }

} // namespace data
} // namespace sensor

namespace actors {

  std::ostream &operator<<(std::ostream &out, const ActorAttribute &attr) {
    using Type = carla::rpc::ActorAttributeType;
    static_assert(static_cast<uint8_t>(Type::SIZE) == 6u, "Please update this function.");
    out << "ActorAttribute(id=" << attr.GetId();
    switch (attr.GetType()) {
      case Type::Bool:
        out << ",type=bool,value=" << (attr.As<bool>() ? "True" : "False");
        break;
      case Type::Int:
        out << ",type=int,value=" << attr.As<int>();
        break;
      case Type::Float:
        out << ",type=float,value=" << attr.As<float>();
        break;
      case Type::String:
        out << ",type=str,value=" << attr.As<std::string>();
        break;
      case Type::RGBColor:
        out << ",type=Color,value=" << attr.As<sensor::data::Color>();
        break;
      default:
        out << ",INVALID";
    }
    if (!attr.IsModifiable()) {
      out << "(const)";
    }
    out << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const ActorBlueprint &bp) {
    out << "ActorBlueprint(id=" << bp.GetId() << ",tags=" << bp.GetTags() << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const BlueprintLibrary &blueprints) {
    return PrintList(out, blueprints);
  }

} // namespace actors
} // namespace carla

void export_blueprint() {
  using namespace boost::python;
  namespace ca = carla::actors;
  namespace crpc = carla::rpc;
  namespace csd = carla::sensor::data;

  enum_<crpc::ActorAttributeType>("ActorAttributeType")
    .value("Bool", crpc::ActorAttributeType::Bool)
    .value("Int", crpc::ActorAttributeType::Int)
    .value("Float", crpc::ActorAttributeType::Float)
    .value("String", crpc::ActorAttributeType::String)
    .value("RGBColor", crpc::ActorAttributeType::RGBColor)
  ;

  class_<csd::Color>("Color")
    .def(init<uint8_t, uint8_t, uint8_t, uint8_t>(
        (arg("r")=0, arg("g")=0, arg("b")=0, arg("a")=255)))
    .def_readwrite("r", &csd::Color::r)
    .def_readwrite("g", &csd::Color::g)
    .def_readwrite("b", &csd::Color::b)
    .def_readwrite("a", &csd::Color::a)
    .def("__eq__", &csd::Color::operator==)
    .def("__ne__", &csd::Color::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<crpc::FloatColor>("FloatColor")
    .def(init<float, float, float, float>(
        (arg("r")=0, arg("g")=0.f, arg("b")=0.f, arg("a")=1.0f)))
    .def_readwrite("r", &crpc::FloatColor::r)
    .def_readwrite("g", &crpc::FloatColor::g)
    .def_readwrite("b", &crpc::FloatColor::b)
    .def_readwrite("a", &crpc::FloatColor::a)
    .def("__eq__", &crpc::FloatColor::operator==)
    .def("__ne__", &crpc::FloatColor::operator!=)
  ;

  class_<csd::OpticalFlowPixel>("OpticalFlowPixel")
    .def(init<float, float>(
        (arg("x")=0, arg("y")=0)))
    .def_readwrite("x", &csd::OpticalFlowPixel::x)
    .def_readwrite("y", &csd::OpticalFlowPixel::y)
    .def("__eq__", &csd::OpticalFlowPixel::operator==)
    .def("__ne__", &csd::OpticalFlowPixel::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ca::ActorAttribute>("ActorAttribute", no_init)
    .add_property("id", CALL_RETURNING_COPY(ca::ActorAttribute, GetId))
    .add_property("type", &ca::ActorAttribute::GetType)
    .add_property("recommended_values", CALL_RETURNING_LIST(ca::ActorAttribute, GetRecommendedValues))
    .add_property("is_modifiable", &ca::ActorAttribute::IsModifiable)
    .def("as_bool", &ca::ActorAttribute::As<bool>)
    .def("as_int", &ca::ActorAttribute::As<int>)
    .def("as_float", &ca::ActorAttribute::As<float>)
    .def("as_str", &ca::ActorAttribute::As<std::string>)
    .def("as_color", &ca::ActorAttribute::As<csd::Color>)
    .def("__eq__", &ca::ActorAttributeValueAccess::operator==<bool>)
    .def("__eq__", &ca::ActorAttributeValueAccess::operator==<int>)
    .def("__eq__", &ca::ActorAttributeValueAccess::operator==<float>)
    .def("__eq__", &ca::ActorAttributeValueAccess::operator==<std::string>)
    .def("__eq__", &ca::ActorAttributeValueAccess::operator==<csd::Color>)
    .def("__eq__", &ca::ActorAttributeValueAccess::operator==<ca::ActorAttribute>)
    .def("__ne__", &ca::ActorAttributeValueAccess::operator!=<bool>)
    .def("__ne__", &ca::ActorAttributeValueAccess::operator!=<int>)
    .def("__ne__", &ca::ActorAttributeValueAccess::operator!=<float>)
    .def("__ne__", &ca::ActorAttributeValueAccess::operator!=<std::string>)
    .def("__ne__", &ca::ActorAttributeValueAccess::operator!=<csd::Color>)
    .def("__ne__", &ca::ActorAttributeValueAccess::operator!=<ca::ActorAttribute>)
    .def("__nonzero__", &ca::ActorAttribute::As<bool>)
    .def("__bool__", &ca::ActorAttribute::As<bool>)
    .def("__int__", &ca::ActorAttribute::As<int>)
    .def("__float__", &ca::ActorAttribute::As<float>)
    .def("__str__", &ca::ActorAttribute::As<std::string>)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ca::ActorBlueprint>("ActorBlueprint", no_init)
    .add_property("id", CALL_RETURNING_COPY(ca::ActorBlueprint, GetId))
    .add_property("tags", CALL_RETURNING_LIST(ca::ActorBlueprint, GetTags))
    .def("has_tag", &ca::ActorBlueprint::ContainsTag)
    .def("match_tags", &ca::ActorBlueprint::MatchTags)
    .def("has_attribute", &ca::ActorBlueprint::ContainsAttribute)
    .def("get_attribute", CALL_RETURNING_COPY_1(ca::ActorBlueprint, GetAttribute, const std::string &))
    .def("set_attribute", &ca::ActorBlueprint::SetAttribute)
    .def("__len__", &ca::ActorBlueprint::size)
    .def("__iter__", range(&ca::ActorBlueprint::begin, &ca::ActorBlueprint::end))
    .def(self_ns::str(self_ns::self))
  ;

  class_<ca::BlueprintLibrary, boost::noncopyable, boost::shared_ptr<ca::BlueprintLibrary>>("BlueprintLibrary", no_init)
    .def("find", +[](const ca::BlueprintLibrary &self, const std::string &key) -> ca::ActorBlueprint {
      return self.at(key);
    }, (arg("id")))
    .def("filter", &ca::BlueprintLibrary::Filter, (arg("wildcard_pattern")))
    .def("filter_by_attribute", &ca::BlueprintLibrary::FilterByAttribute, (arg("name"), arg("value")))
    .def("__getitem__", +[](const ca::BlueprintLibrary &self, size_t pos) -> ca::ActorBlueprint {
      return self.at(pos);
    })
    .def("__len__", &ca::BlueprintLibrary::size)
    .def("__iter__", range(&ca::BlueprintLibrary::begin, &ca::BlueprintLibrary::end))
    .def(self_ns::str(self_ns::self))
  ;
}
