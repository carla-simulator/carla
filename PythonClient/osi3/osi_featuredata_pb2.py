# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: osi_featuredata.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from osi3 import osi_version_pb2 as osi3_dot_osi__version__pb2
from osi3 import osi_common_pb2 as osi3_dot_osi__common__pb2


DESCRIPTOR = _descriptor.FileDescriptor(
  name='osi_featuredata.proto',
  package='osi3',
  syntax='proto3',
  serialized_options=_b('H\001'),
  serialized_pb=_b('\n\x15osi_featuredata.proto\x12\x04osi3\x1a\x16osi3/osi_version.proto\x1a\x15osi3/osi_common.proto\"\x96\x01\n\x0b\x46\x65\x61tureData\x12\'\n\x07version\x18\x01 \x01(\x0b\x32\x16.osi3.InterfaceVersion\x12.\n\x0cradar_sensor\x18\x02 \x03(\x0b\x32\x18.osi3.RadarDetectionData\x12.\n\x0clidar_sensor\x18\x03 \x03(\x0b\x32\x18.osi3.LidarDetectionData\"\xc4\x04\n\x15SensorDetectionHeader\x12)\n\x10measurement_time\x18\x01 \x01(\x0b\x32\x0f.osi3.Timestamp\x12\x15\n\rcycle_counter\x18\x02 \x01(\x04\x12\x31\n\x11mounting_position\x18\x03 \x01(\x0b\x32\x16.osi3.MountingPosition\x12\x36\n\x16mounting_position_rmse\x18\x04 \x01(\x0b\x32\x16.osi3.MountingPosition\x12\x41\n\x0e\x64\x61ta_qualifier\x18\x05 \x01(\x0e\x32).osi3.SensorDetectionHeader.DataQualifier\x12\"\n\x1anumber_of_valid_detections\x18\x06 \x01(\r\x12#\n\tsensor_id\x18\x07 \x01(\x0b\x32\x10.osi3.Identifier\"\xf1\x01\n\rDataQualifier\x12\x1a\n\x16\x44\x41TA_QUALIFIER_UNKNOWN\x10\x00\x12\x18\n\x14\x44\x41TA_QUALIFIER_OTHER\x10\x01\x12\x1c\n\x18\x44\x41TA_QUALIFIER_AVAILABLE\x10\x02\x12$\n DATA_QUALIFIER_AVAILABLE_REDUCED\x10\x03\x12 \n\x1c\x44\x41TA_QUALIFIER_NOT_AVAILABLE\x10\x04\x12\x1c\n\x18\x44\x41TA_QUALIFIER_BLINDNESS\x10\x05\x12&\n\"DATA_QUALIFIER_TEMPORARY_AVAILABLE\x10\x06\"j\n\x12RadarDetectionData\x12+\n\x06header\x18\x01 \x01(\x0b\x32\x1b.osi3.SensorDetectionHeader\x12\'\n\tdetection\x18\x02 \x03(\x0b\x32\x14.osi3.RadarDetection\"\xf5\x02\n\x0eRadarDetection\x12\x1d\n\x15\x65xistence_probability\x18\x01 \x01(\x01\x12#\n\tobject_id\x18\x02 \x01(\x0b\x32\x10.osi3.Identifier\x12#\n\x08position\x18\x03 \x01(\x0b\x32\x11.osi3.Spherical3d\x12(\n\rposition_rmse\x18\x04 \x01(\x0b\x32\x11.osi3.Spherical3d\x12\x17\n\x0fradial_velocity\x18\x05 \x01(\x01\x12\x1c\n\x14radial_velocity_rmse\x18\x06 \x01(\x01\x12\x0b\n\x03rcs\x18\x07 \x01(\x01\x12\x0b\n\x03snr\x18\x08 \x01(\x01\x12 \n\x18point_target_probability\x18\t \x01(\x01\x12&\n\x0c\x61mbiguity_id\x18\n \x01(\x0b\x32\x10.osi3.Identifier\x12\x35\n\x0e\x63lassification\x18\x0b \x01(\x0e\x32\x1d.osi3.DetectionClassification\"j\n\x12LidarDetectionData\x12+\n\x06header\x18\x01 \x01(\x0b\x32\x1b.osi3.SensorDetectionHeader\x12\'\n\tdetection\x18\x02 \x03(\x0b\x32\x14.osi3.LidarDetection\"\xc8\x02\n\x0eLidarDetection\x12\x1d\n\x15\x65xistence_probability\x18\x01 \x01(\x01\x12#\n\tobject_id\x18\x02 \x01(\x0b\x32\x10.osi3.Identifier\x12#\n\x08position\x18\x03 \x01(\x0b\x32\x11.osi3.Spherical3d\x12(\n\rposition_rmse\x18\x04 \x01(\x0b\x32\x11.osi3.Spherical3d\x12\x0e\n\x06height\x18\x05 \x01(\x01\x12\x13\n\x0bheight_rmse\x18\x06 \x01(\x01\x12\x11\n\tintensity\x18\x07 \x01(\x01\x12\x1e\n\x16\x66ree_space_probability\x18\x08 \x01(\x01\x12\x35\n\x0e\x63lassification\x18\t \x01(\x0e\x32\x1d.osi3.DetectionClassification\x12\x14\n\x0creflectivity\x18\n \x01(\x01*\x86\x02\n\x17\x44\x65tectionClassification\x12$\n DETECTION_CLASSIFICATION_UNKNOWN\x10\x00\x12\"\n\x1e\x44\x45TECTION_CLASSIFICATION_OTHER\x10\x01\x12$\n DETECTION_CLASSIFICATION_INVALID\x10\x02\x12$\n DETECTION_CLASSIFICATION_CLUTTER\x10\x03\x12)\n%DETECTION_CLASSIFICATION_OVERDRIVABLE\x10\x04\x12*\n&DETECTION_CLASSIFICATION_UNDERDRIVABLE\x10\x05\x42\x02H\x01\x62\x06proto3')
  ,
  dependencies=[osi3_dot_osi__version__pb2.DESCRIPTOR,osi3_dot_osi__common__pb2.DESCRIPTOR,])

_DETECTIONCLASSIFICATION = _descriptor.EnumDescriptor(
  name='DetectionClassification',
  full_name='osi3.DetectionClassification',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='DETECTION_CLASSIFICATION_UNKNOWN', index=0, number=0,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DETECTION_CLASSIFICATION_OTHER', index=1, number=1,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DETECTION_CLASSIFICATION_INVALID', index=2, number=2,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DETECTION_CLASSIFICATION_CLUTTER', index=3, number=3,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DETECTION_CLASSIFICATION_OVERDRIVABLE', index=4, number=4,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DETECTION_CLASSIFICATION_UNDERDRIVABLE', index=5, number=5,
      serialized_options=None,
      type=None),
  ],
  containing_type=None,
  serialized_options=None,
  serialized_start=1738,
  serialized_end=2000,
)
_sym_db.RegisterEnumDescriptor(_DETECTIONCLASSIFICATION)

DetectionClassification = enum_type_wrapper.EnumTypeWrapper(_DETECTIONCLASSIFICATION)
DETECTION_CLASSIFICATION_UNKNOWN = 0
DETECTION_CLASSIFICATION_OTHER = 1
DETECTION_CLASSIFICATION_INVALID = 2
DETECTION_CLASSIFICATION_CLUTTER = 3
DETECTION_CLASSIFICATION_OVERDRIVABLE = 4
DETECTION_CLASSIFICATION_UNDERDRIVABLE = 5


_SENSORDETECTIONHEADER_DATAQUALIFIER = _descriptor.EnumDescriptor(
  name='DataQualifier',
  full_name='osi3.SensorDetectionHeader.DataQualifier',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='DATA_QUALIFIER_UNKNOWN', index=0, number=0,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DATA_QUALIFIER_OTHER', index=1, number=1,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DATA_QUALIFIER_AVAILABLE', index=2, number=2,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DATA_QUALIFIER_AVAILABLE_REDUCED', index=3, number=3,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DATA_QUALIFIER_NOT_AVAILABLE', index=4, number=4,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DATA_QUALIFIER_BLINDNESS', index=5, number=5,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DATA_QUALIFIER_TEMPORARY_AVAILABLE', index=6, number=6,
      serialized_options=None,
      type=None),
  ],
  containing_type=None,
  serialized_options=None,
  serialized_start=571,
  serialized_end=812,
)
_sym_db.RegisterEnumDescriptor(_SENSORDETECTIONHEADER_DATAQUALIFIER)


_FEATUREDATA = _descriptor.Descriptor(
  name='FeatureData',
  full_name='osi3.FeatureData',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='version', full_name='osi3.FeatureData.version', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='radar_sensor', full_name='osi3.FeatureData.radar_sensor', index=1,
      number=2, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='lidar_sensor', full_name='osi3.FeatureData.lidar_sensor', index=2,
      number=3, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=79,
  serialized_end=229,
)


_SENSORDETECTIONHEADER = _descriptor.Descriptor(
  name='SensorDetectionHeader',
  full_name='osi3.SensorDetectionHeader',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='measurement_time', full_name='osi3.SensorDetectionHeader.measurement_time', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='cycle_counter', full_name='osi3.SensorDetectionHeader.cycle_counter', index=1,
      number=2, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='mounting_position', full_name='osi3.SensorDetectionHeader.mounting_position', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='mounting_position_rmse', full_name='osi3.SensorDetectionHeader.mounting_position_rmse', index=3,
      number=4, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='data_qualifier', full_name='osi3.SensorDetectionHeader.data_qualifier', index=4,
      number=5, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='number_of_valid_detections', full_name='osi3.SensorDetectionHeader.number_of_valid_detections', index=5,
      number=6, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='sensor_id', full_name='osi3.SensorDetectionHeader.sensor_id', index=6,
      number=7, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
    _SENSORDETECTIONHEADER_DATAQUALIFIER,
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=232,
  serialized_end=812,
)


_RADARDETECTIONDATA = _descriptor.Descriptor(
  name='RadarDetectionData',
  full_name='osi3.RadarDetectionData',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='header', full_name='osi3.RadarDetectionData.header', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='detection', full_name='osi3.RadarDetectionData.detection', index=1,
      number=2, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=814,
  serialized_end=920,
)


_RADARDETECTION = _descriptor.Descriptor(
  name='RadarDetection',
  full_name='osi3.RadarDetection',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='existence_probability', full_name='osi3.RadarDetection.existence_probability', index=0,
      number=1, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='object_id', full_name='osi3.RadarDetection.object_id', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='position', full_name='osi3.RadarDetection.position', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='position_rmse', full_name='osi3.RadarDetection.position_rmse', index=3,
      number=4, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='radial_velocity', full_name='osi3.RadarDetection.radial_velocity', index=4,
      number=5, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='radial_velocity_rmse', full_name='osi3.RadarDetection.radial_velocity_rmse', index=5,
      number=6, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='rcs', full_name='osi3.RadarDetection.rcs', index=6,
      number=7, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='snr', full_name='osi3.RadarDetection.snr', index=7,
      number=8, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='point_target_probability', full_name='osi3.RadarDetection.point_target_probability', index=8,
      number=9, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='ambiguity_id', full_name='osi3.RadarDetection.ambiguity_id', index=9,
      number=10, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='classification', full_name='osi3.RadarDetection.classification', index=10,
      number=11, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=923,
  serialized_end=1296,
)


_LIDARDETECTIONDATA = _descriptor.Descriptor(
  name='LidarDetectionData',
  full_name='osi3.LidarDetectionData',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='header', full_name='osi3.LidarDetectionData.header', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='detection', full_name='osi3.LidarDetectionData.detection', index=1,
      number=2, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=1298,
  serialized_end=1404,
)


_LIDARDETECTION = _descriptor.Descriptor(
  name='LidarDetection',
  full_name='osi3.LidarDetection',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='existence_probability', full_name='osi3.LidarDetection.existence_probability', index=0,
      number=1, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='object_id', full_name='osi3.LidarDetection.object_id', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='position', full_name='osi3.LidarDetection.position', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='position_rmse', full_name='osi3.LidarDetection.position_rmse', index=3,
      number=4, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='height', full_name='osi3.LidarDetection.height', index=4,
      number=5, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='height_rmse', full_name='osi3.LidarDetection.height_rmse', index=5,
      number=6, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='intensity', full_name='osi3.LidarDetection.intensity', index=6,
      number=7, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='free_space_probability', full_name='osi3.LidarDetection.free_space_probability', index=7,
      number=8, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='classification', full_name='osi3.LidarDetection.classification', index=8,
      number=9, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='reflectivity', full_name='osi3.LidarDetection.reflectivity', index=9,
      number=10, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=1407,
  serialized_end=1735,
)

_FEATUREDATA.fields_by_name['version'].message_type = osi3_dot_osi__version__pb2._INTERFACEVERSION
_FEATUREDATA.fields_by_name['radar_sensor'].message_type = _RADARDETECTIONDATA
_FEATUREDATA.fields_by_name['lidar_sensor'].message_type = _LIDARDETECTIONDATA
_SENSORDETECTIONHEADER.fields_by_name['measurement_time'].message_type = osi3_dot_osi__common__pb2._TIMESTAMP
_SENSORDETECTIONHEADER.fields_by_name['mounting_position'].message_type = osi3_dot_osi__common__pb2._MOUNTINGPOSITION
_SENSORDETECTIONHEADER.fields_by_name['mounting_position_rmse'].message_type = osi3_dot_osi__common__pb2._MOUNTINGPOSITION
_SENSORDETECTIONHEADER.fields_by_name['data_qualifier'].enum_type = _SENSORDETECTIONHEADER_DATAQUALIFIER
_SENSORDETECTIONHEADER.fields_by_name['sensor_id'].message_type = osi3_dot_osi__common__pb2._IDENTIFIER
_SENSORDETECTIONHEADER_DATAQUALIFIER.containing_type = _SENSORDETECTIONHEADER
_RADARDETECTIONDATA.fields_by_name['header'].message_type = _SENSORDETECTIONHEADER
_RADARDETECTIONDATA.fields_by_name['detection'].message_type = _RADARDETECTION
_RADARDETECTION.fields_by_name['object_id'].message_type = osi3_dot_osi__common__pb2._IDENTIFIER
_RADARDETECTION.fields_by_name['position'].message_type = osi3_dot_osi__common__pb2._SPHERICAL3D
_RADARDETECTION.fields_by_name['position_rmse'].message_type = osi3_dot_osi__common__pb2._SPHERICAL3D
_RADARDETECTION.fields_by_name['ambiguity_id'].message_type = osi3_dot_osi__common__pb2._IDENTIFIER
_RADARDETECTION.fields_by_name['classification'].enum_type = _DETECTIONCLASSIFICATION
_LIDARDETECTIONDATA.fields_by_name['header'].message_type = _SENSORDETECTIONHEADER
_LIDARDETECTIONDATA.fields_by_name['detection'].message_type = _LIDARDETECTION
_LIDARDETECTION.fields_by_name['object_id'].message_type = osi3_dot_osi__common__pb2._IDENTIFIER
_LIDARDETECTION.fields_by_name['position'].message_type = osi3_dot_osi__common__pb2._SPHERICAL3D
_LIDARDETECTION.fields_by_name['position_rmse'].message_type = osi3_dot_osi__common__pb2._SPHERICAL3D
_LIDARDETECTION.fields_by_name['classification'].enum_type = _DETECTIONCLASSIFICATION
DESCRIPTOR.message_types_by_name['FeatureData'] = _FEATUREDATA
DESCRIPTOR.message_types_by_name['SensorDetectionHeader'] = _SENSORDETECTIONHEADER
DESCRIPTOR.message_types_by_name['RadarDetectionData'] = _RADARDETECTIONDATA
DESCRIPTOR.message_types_by_name['RadarDetection'] = _RADARDETECTION
DESCRIPTOR.message_types_by_name['LidarDetectionData'] = _LIDARDETECTIONDATA
DESCRIPTOR.message_types_by_name['LidarDetection'] = _LIDARDETECTION
DESCRIPTOR.enum_types_by_name['DetectionClassification'] = _DETECTIONCLASSIFICATION
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

FeatureData = _reflection.GeneratedProtocolMessageType('FeatureData', (_message.Message,), dict(
  DESCRIPTOR = _FEATUREDATA,
  __module__ = 'osi_featuredata_pb2'
  # @@protoc_insertion_point(class_scope:osi3.FeatureData)
  ))
_sym_db.RegisterMessage(FeatureData)

SensorDetectionHeader = _reflection.GeneratedProtocolMessageType('SensorDetectionHeader', (_message.Message,), dict(
  DESCRIPTOR = _SENSORDETECTIONHEADER,
  __module__ = 'osi_featuredata_pb2'
  # @@protoc_insertion_point(class_scope:osi3.SensorDetectionHeader)
  ))
_sym_db.RegisterMessage(SensorDetectionHeader)

RadarDetectionData = _reflection.GeneratedProtocolMessageType('RadarDetectionData', (_message.Message,), dict(
  DESCRIPTOR = _RADARDETECTIONDATA,
  __module__ = 'osi_featuredata_pb2'
  # @@protoc_insertion_point(class_scope:osi3.RadarDetectionData)
  ))
_sym_db.RegisterMessage(RadarDetectionData)

RadarDetection = _reflection.GeneratedProtocolMessageType('RadarDetection', (_message.Message,), dict(
  DESCRIPTOR = _RADARDETECTION,
  __module__ = 'osi_featuredata_pb2'
  # @@protoc_insertion_point(class_scope:osi3.RadarDetection)
  ))
_sym_db.RegisterMessage(RadarDetection)

LidarDetectionData = _reflection.GeneratedProtocolMessageType('LidarDetectionData', (_message.Message,), dict(
  DESCRIPTOR = _LIDARDETECTIONDATA,
  __module__ = 'osi_featuredata_pb2'
  # @@protoc_insertion_point(class_scope:osi3.LidarDetectionData)
  ))
_sym_db.RegisterMessage(LidarDetectionData)

LidarDetection = _reflection.GeneratedProtocolMessageType('LidarDetection', (_message.Message,), dict(
  DESCRIPTOR = _LIDARDETECTION,
  __module__ = 'osi_featuredata_pb2'
  # @@protoc_insertion_point(class_scope:osi3.LidarDetection)
  ))
_sym_db.RegisterMessage(LidarDetection)


DESCRIPTOR._options = None
# @@protoc_insertion_point(module_scope)
