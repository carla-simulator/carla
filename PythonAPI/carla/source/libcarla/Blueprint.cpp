// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/BlueprintLibrary.h>
#include <carla/client/ActorBlueprint.h>

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

} // namespace data
} // namespace sensor

namespace client {

  std::ostream &operator<<(std::ostream &out, const ActorAttribute &attr) {
    using Type = carla::rpc::ActorAttributeType;
    static_assert(static_cast<uint8_t>(Type::SIZE) == 5u, "Please update this function.");
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

} // namespace client
} // namespace carla

void export_blueprint() {
  using namespace boost::python;
  namespace cc = carla::client;
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
        (arg("r")=0, arg("g")=0, arg("b")=0, arg("a")=255), "@DocString(Color.__init__)"))
    .def_readwrite("r", &csd::Color::r, "@DocString(Color.r)")
    .def_readwrite("g", &csd::Color::g, "@DocString(Color.g)")
    .def_readwrite("b", &csd::Color::b, "@DocString(Color.b)")
    .def_readwrite("a", &csd::Color::a, "@DocString(Color.a)")
    .def("__eq__", &csd::Color::operator==, "@DocString(Color.__eq__)")
    .def("__ne__", &csd::Color::operator!=, "@DocString(Color.__ne__)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::ActorAttribute>("ActorAttribute", no_init)
    .add_property("id", CALL_RETURNING_COPY(cc::ActorAttribute, GetId), "@DocString(ActorAttribute.id)")
    .add_property("type", &cc::ActorAttribute::GetType, "@DocString(ActorAttribute.type)")
    .add_property("recommended_values", CALL_RETURNING_LIST(cc::ActorAttribute, GetRecommendedValues), "@DocString(ActorAttribute.recommended_values)")
    .add_property("is_modifiable", &cc::ActorAttribute::IsModifiable, "@DocString(ActorAttribute.is_modifiable)")
    .def("as_bool", &cc::ActorAttribute::As<bool>, "@DocString(ActorAttribute.as_bool)")
    .def("as_int", &cc::ActorAttribute::As<int>, "@DocString(ActorAttribute.as_int)")
    .def("as_float", &cc::ActorAttribute::As<float>, "@DocString(ActorAttribute.as_float)")
    .def("as_str", &cc::ActorAttribute::As<std::string>, "@DocString(ActorAttribute.as_str)")
    .def("as_color", &cc::ActorAttribute::As<csd::Color>, "@DocString(ActorAttribute.as_color)")
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<bool>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<int>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<float>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<std::string>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<csd::Color>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<cc::ActorAttribute>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<bool>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<int>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<float>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<std::string>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<csd::Color>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<cc::ActorAttribute>)
    .def("__nonzero__", &cc::ActorAttribute::As<bool>)
    .def("__bool__", &cc::ActorAttribute::As<bool>)
    .def("__int__", &cc::ActorAttribute::As<int>)
    .def("__float__", &cc::ActorAttribute::As<float>)
    .def("__str__", &cc::ActorAttribute::As<std::string>)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::ActorBlueprint>("ActorBlueprint", no_init)
    .add_property("id", CALL_RETURNING_COPY(cc::ActorBlueprint, GetId), "@DocString(ActorBlueprint.id)")
    .add_property("tags", CALL_RETURNING_LIST(cc::ActorBlueprint, GetTags), "@DocString(ActorBlueprint.tags)")
    .def("has_tag", &cc::ActorBlueprint::ContainsTag, "@DocString(ActorBlueprint.has_tag)")
    .def("match_tags", &cc::ActorBlueprint::MatchTags, "@DocString(ActorBlueprint.match_tags)")
    .def("has_attribute", &cc::ActorBlueprint::ContainsAttribute, "@DocString(ActorBlueprint.has_attribute)")
    .def("get_attribute", CALL_RETURNING_COPY_1(cc::ActorBlueprint, GetAttribute, const std::string &), "@DocString(ActorBlueprint.get_attribute)")
    .def("set_attribute", &cc::ActorBlueprint::SetAttribute, "@DocString(ActorBlueprint.set_attribute)")
    .def("__len__", &cc::ActorBlueprint::size, "@DocString(ActorBlueprint.__len__)")
    .def("__iter__", range(&cc::ActorBlueprint::begin, &cc::ActorBlueprint::end), "@DocString(ActorBlueprint.__iter__)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::BlueprintLibrary, boost::noncopyable, boost::shared_ptr<cc::BlueprintLibrary>>("BlueprintLibrary", no_init)
    .def("find", +[](const cc::BlueprintLibrary &self, const std::string &key) -> cc::ActorBlueprint {
      return self.at(key);
    }, (arg("id")), "@DocString(BlueprintLibrary.find)")
    .def("filter", &cc::BlueprintLibrary::Filter, (arg("wildcard_pattern")), "@DocString(BlueprintLibrary.filter)")
    .def("__getitem__", +[](const cc::BlueprintLibrary &self, size_t pos) -> cc::ActorBlueprint {
      return self.at(pos);
    }, "@DocString(BlueprintLibrary.__getitem__)")
    .def("__len__", &cc::BlueprintLibrary::size, "@DocString(BlueprintLibrary.__len__)")
    .def("__iter__", range(&cc::BlueprintLibrary::begin, &cc::BlueprintLibrary::end), "@DocString(BlueprintLibrary.__iter__)")
    .def(self_ns::str(self_ns::self))
  ;
}
