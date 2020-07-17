// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>

// get the final path + filename
std::string GetRecorderFilename(std::string Filename);

// ---------
// recorder
// ---------

// write binary data (using sizeof())
template <typename T>
void WriteValue(std::ofstream &OutFile, const T &InObj)
{
  OutFile.write(reinterpret_cast<const char *>(&InObj), sizeof(T));
}

template <typename T>
void WriteStdVector(std::ofstream &OutFile, const std::vector<T> &InVec)
{
  WriteValue<uint32_t>(OutFile, InVec.size());
  for (const auto& InObj : InVec)
  {
    WriteValue<T>(OutFile, InObj);
  }
}

template <typename T>
void WriteTArray(std::ofstream &OutFile, const TArray<T> &InVec)
{
  WriteValue<uint32_t>(OutFile, InVec.Num());
  for (const auto& InObj : InVec)
  {
    WriteValue<T>(OutFile, InObj);
  }
}

// write binary data from FVector
void WriteFVector(std::ofstream &OutFile, const FVector &InObj);

// write binary data from FTransform
// void WriteFTransform(std::ofstream &OutFile, const FTransform &InObj);
// write binary data from FString (length + text)
void WriteFString(std::ofstream &OutFile, const FString &InObj);

// ---------
// replayer
// ---------

// read binary data (using sizeof())
template <typename T>
void ReadValue(std::ifstream &InFile, T &OutObj)
{
  InFile.read(reinterpret_cast<char *>(&OutObj), sizeof(T));
}

template <typename T>
void ReadStdVector(std::ifstream &InFile, std::vector<T> &OutVec)
{
  uint32_t VecSize;
  ReadValue<uint32_t>(InFile, VecSize);
  OutVec.clear();
  for (uint32_t i = 0; i < VecSize; ++i)
  {
    T InObj;
    ReadValue<T>(InFile, InObj);
    OutVec.push_back(InObj);
  }
}

template <typename T>
void ReadTArray(std::ifstream &InFile, TArray<T> &OutVec)
{
  uint32_t VecSize;
  ReadValue<uint32_t>(InFile, VecSize);
  OutVec.Empty();
  for (uint32_t i = 0; i < VecSize; ++i)
  {
    T InObj;
    ReadValue<T>(InFile, InObj);
    OutVec.Add(InObj);
  }
}

// read binary data from FVector
void ReadFVector(std::ifstream &InFile, FVector &OutObj);

// read binary data from FTransform
// void ReadTransform(std::ifstream &InFile, FTransform &OutObj);
// read binary data from FString (length + text)
void ReadFString(std::ifstream &InFile, FString &OutObj);
