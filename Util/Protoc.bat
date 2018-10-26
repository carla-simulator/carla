@echo off

set PROTOBUF_SRC_DIR=Proto
set PROTOBUF_CPP_OUT_DIR=CarlaServer/source/carla/server
set PROTOBUF_PY_OUT_DIR=../Deprecated/PythonClient/carla
set PROTO_BASENAME=carla_server

if "%1" == "--clean" (
  rem Delete existing ones.
  @rd /s /q "%PROTOBUF_CPP_OUT_DIR%\carla_server.pb.h" 2>nul
  @rd /s /q "%PROTOBUF_CPP_OUT_DIR%\carla_server.pb.cc" 2>nul
  @rd /s /q "%PROTOBUF_PY_OUT_DIR%\carla_server_pb2.py" 2>nul
  goto end
)

set PROTOC=Build\protobuf-install\bin\protoc.exe

if exist %PROTOC% (

  echo Compiling %PROTO_BASENAME%.proto...

  %PROTOC% ^
      -I=%PROTOBUF_SRC_DIR% ^
      --cpp_out=%PROTOBUF_CPP_OUT_DIR% ^
      --python_out=%PROTOBUF_PY_OUT_DIR% ^
    %PROTOBUF_SRC_DIR%/%PROTO_BASENAME%.proto

  echo done.

) else (

  echo ERROR: Missing protobuf compiler: %PROTOC%
  exit /b

)

:end
