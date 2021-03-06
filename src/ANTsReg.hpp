/*
   ANTsReg.hpp

   Author:      Benjamin A. Thomas

   Copyright 2018 Institute of Nuclear Medicine, University College London.
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   
 */

#pragma once

#ifndef _ANTSREG_HPP_
#define _ANTSREG_HPP_ 

#include <nlohmann/json.hpp>
#include <boost/filesystem.hpp>
#include <glog/logging.h>

#include <string>

#include <itkImageFileReader.h>

#include <antsRegistrationTemplateHeader.h>
#include <include/ants.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>


namespace reg {

template <class TImage>
class ANTsReg
{

public:
  typedef TImage ImageType;

  //DicomReader();
  ANTsReg();

  void SetParams(const nlohmann::json &params);
  void SetOutputDirectory(const boost::filesystem::path outDir);
  void SetOutputPrefix(const std::string &s){ _prefix = s; };
  void SetReferenceFileName(const boost::filesystem::path refFileName);
  void SetFloatingFileName(const boost::filesystem::path floatFileName);

  void Update();

  typename TImage::ConstPointer GetOutputImage();
  typename TImage::ConstPointer GetOutputInverseImage();

protected:

  void InsertParam(const std::string &key, const std::string &info);

  std::string _prefix;
  boost::filesystem::path _outDir;
  boost::filesystem::path _refFileName;
  boost::filesystem::path _floatFileName;

  nlohmann::json _regParams;
  bool _bDefaultParams = true;

  const std::string _defaultArgs = "3 -m CC[<%%REF%%>,<%%FLOAT%%>,1,4] -i 10x5x2 -o <%%PREFIX%%> -t SyN[0.5] -r Gauss[3,0] -G";

  std::string _argList;

};

//Constructor
template <typename TImage>
ANTsReg<TImage>::ANTsReg()
{

  DLOG(INFO) << "Initialised ANTsReg.";

  //Use defaults unless set otherwise via SetParams;
  _argList = _defaultArgs;

}

template <typename TImage>
typename TImage::ConstPointer ANTsReg<TImage>::GetOutputImage(){

  boost::filesystem::path tImagePath = _outDir;
  tImagePath /= _prefix;
  tImagePath += "Warped.nii.gz";

  typedef typename itk::ImageFileReader<TImage> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  try {
    reader->SetFileName( tImagePath.string() );
    reader->Update();
    LOG(INFO) << "Read " << tImagePath << " successfully.";
  }
  catch (itk::ExceptionObject &ex)
  {
    LOG(ERROR) << ex;
    LOG(ERROR) << "Unable to read image " << tImagePath;
    throw false;
  }

  return static_cast< const TImage * >( reader->GetOutput() );
}

template <typename TImage>
typename TImage::ConstPointer ANTsReg<TImage>::GetOutputInverseImage(){

  boost::filesystem::path iImagePath = _outDir;
  iImagePath /= _prefix;
  iImagePath += "InverseWarped.nii.gz";

  typedef typename itk::ImageFileReader<TImage> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  try {
    reader->SetFileName( iImagePath.string() );
    reader->Update();
    LOG(INFO) << "Read " << iImagePath << " successfully.";
  }
  catch (itk::ExceptionObject &ex)
  {
    LOG(ERROR) << ex;
    LOG(ERROR) << "Unable to read image " << iImagePath;
    throw false;
  }

  return static_cast< const TImage * >( reader->GetOutput() );
}

template <typename TImage>
void ANTsReg<TImage>::SetOutputDirectory(const boost::filesystem::path outDir){

  if ( boost::filesystem::exists(outDir) ){
    if (!boost::filesystem::is_directory(outDir)){
      LOG(ERROR) << "Output directory: " << outDir << "does not appear to be a directory!";
      throw(false);
    }
  } else {
    try {
      boost::filesystem::create_directories(outDir);
    } catch (const boost::filesystem::filesystem_error &e){
      LOG(ERROR) << "Cannot create output folder : " << outDir;
      throw(false);
    }
  }

  _outDir = outDir;

}

template <typename TImage>
void ANTsReg<TImage>::SetReferenceFileName(const boost::filesystem::path refFileName){

  try {
    if (!boost::filesystem::is_regular_file(refFileName)){
      LOG(ERROR) << "Cannot find reference image file : " << refFileName;
      throw(false);      
    }
  } catch (const boost::filesystem::filesystem_error &e){
      LOG(ERROR) << "File system error reading : " << refFileName;
      throw(false);
    }
  
  _refFileName = refFileName;
  
}

template <typename TImage>
void ANTsReg<TImage>::SetFloatingFileName(const boost::filesystem::path floatFileName){

  try {
    if (!boost::filesystem::is_regular_file(floatFileName)){
      LOG(ERROR) << "Cannot find floating image file : " << floatFileName;
      throw(false);      
    }
  } catch (const boost::filesystem::filesystem_error &e){
      LOG(ERROR) << "File system error reading : " << floatFileName;
      throw(false);
    }
  
  _floatFileName = floatFileName;
  
}

template <typename TImage>
void ANTsReg<TImage>::SetParams(const nlohmann::json &params){ 

  _argList = params.get<std::string>();
  _bDefaultParams = false;

};

template <typename TImage>
void ANTsReg<TImage>::InsertParam(const std::string &key, const std::string &info){

  std::string target = "<%%" + key + "%%>";
  std::string::size_type n = _argList.find(target);

  if (n == std::string::npos){
    LOG(WARNING) << "Replacement key: " << target << " not found!";
  }

  boost::replace_all(_argList, target, info);
}

template <typename TImage>
void ANTsReg<TImage>::Update(){

  boost::filesystem::path fullPrefix = _outDir;
  fullPrefix /= _prefix;

  boost::filesystem::path tImagePath = fullPrefix;
  tImagePath += "Warped.nii.gz";

  boost::filesystem::path iImagePath = fullPrefix;
  iImagePath += "InverseWarped.nii.gz";

  InsertParam("FLOAT", _floatFileName.string());
  InsertParam("REF", _refFileName.string());
  InsertParam("PREFIX", fullPrefix.string());
  //InsertParam("WARPEDIMG", tImagePath.string());
  //InsertParam("INVWARPEDIMG", iImagePath.string());

  std::vector<std::string> args;

  boost::split_regex( args, _argList, boost::regex( " " ) ) ;

  if (_bDefaultParams){
    LOG(WARNING) << "No registration parameters set. Using defaults!";
  }

  LOG(INFO) << "Registration parameters:";
  LOG(INFO) << "";
  LOG(INFO) << _argList;
  LOG(INFO) << "";

  std::vector<std::string> finalArgs;

  int x=1;
  for (auto a : args){
    if (a != "") {
      finalArgs.push_back(a);
      DLOG(INFO) << x << "\t\t" << a;
      x++;
    }
  }

  LOG(INFO) << "Starting ANTs registration. This will take a while...";
  google::FlushLogFiles(google::INFO);

  //ants::antsRegistration( finalArgs, &std::cout);
  ants::ANTS( finalArgs, &std::cout);

  LOG(INFO) << "Registration complete!";

  
}

} //namespace reg


#endif