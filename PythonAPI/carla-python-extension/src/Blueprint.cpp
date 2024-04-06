// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

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

  class_<cc::ActorAttribute>("ActorAttribute", no_init)
    .add_property("id", CALL_RETURNING_COPY(cc::ActorAttribute, GetId))
    .add_property("type", &cc::ActorAttribute::GetType)
    .add_property("recommended_values", CALL_RETURNING_LIST(cc::ActorAttribute, GetRecommendedValues))
    .add_property("is_modifiable", &cc::ActorAttribute::IsModifiable)
    .def("as_bool", &cc::ActorAttribute::As<bool>)
    .def("as_int", &cc::ActorAttribute::As<int>)
    .def("as_float", &cc::ActorAttribute::As<float>)
    .def("as_str", &cc::ActorAttribute::As<std::string>)
    .def("as_color", &cc::ActorAttribute::As<csd::Color>)
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
    .add_property("id", CALL_RETURNING_COPY(cc::ActorBlueprint, GetId))
    .add_property("tags", CALL_RETURNING_LIST(cc::ActorBlueprint, GetTags))
    .def("has_tag", &cc::ActorBlueprint::ContainsTag)
    .def("match_tags", &cc::ActorBlueprint::MatchTags)
    .def("has_attribute", &cc::ActorBlueprint::ContainsAttribute)
    .def("get_attribute", CALL_RETURNING_COPY_1(cc::ActorBlueprint, GetAttribute, const std::string &))
    .def("set_attribute", &cc::ActorBlueprint::SetAttribute)
    .def("__len__", &cc::ActorBlueprint::size)
    .def("__iter__", range(&cc::ActorBlueprint::begin, &cc::ActorBlueprint::end))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::BlueprintLibrary, boost::noncopyable, std::shared_ptr<cc::BlueprintLibrary>>("BlueprintLibrary", no_init)
    .def("find", +[](const cc::BlueprintLibrary &self, const std::string &key) -> cc::ActorBlueprint {
      return self.at(key);
    }, (arg("id")))
    .def("filter", &cc::BlueprintLibrary::Filter, (arg("wildcard_pattern")))
    .def("filter_by_attribute", &cc::BlueprintLibrary::FilterByAttribute, (arg("name"), arg("value")))
    .def("__getitem__", +[](const cc::BlueprintLibrary &self, size_t pos) -> cc::ActorBlueprint {
      return self.at(pos);
    })
    .def("__len__", &cc::BlueprintLibrary::size)
    .def("__iter__", range(&cc::BlueprintLibrary::begin, &cc::BlueprintLibrary::end))
    .def(self_ns::str(self_ns::self))
  ;
}
