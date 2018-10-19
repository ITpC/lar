/*
 * Copyright Pavel Kraynyukhov 2018.
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 *             http://www.boost.org/LICENSE_1_0.txt)
 *
 *  $Id: lar:TOC.h Jun 14, 2018 11:35:54 AM $
 *
 *  EMail: pavel.kraynyukhov@gmail.com
 */


#ifndef __TOC_H__
#define __TOC_H__
#include <vector>
#include <memory>
#include <experimental/filesystem>

#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>

#include <bz2Compression.h>
#include <fstream>

#include <utils.h>

namespace fs = std::experimental::filesystem::v1;

namespace lar{
  typedef ::byte sha256sum[CryptoPP::SHA256::DIGESTSIZE];
  
  struct TOCItem
  {
    size_t                  fsize;
    size_t                  compressedSize;
    uint32_t                path_size;
    fs::perms               permissions;
    sha256sum               hashsum;
    fs::path                relativePath;
    
    explicit TOCItem(const fs::path& _path)
    : fsize(fs::file_size(_path)),
      compressedSize(0),
      path_size(_path.u8string().length()),
      permissions(fs::status(_path).permissions()),
      relativePath(_path)
    {
    }
    
    explicit TOCItem(const std::vector<uint8_t>& bytes, const size_t offset=0)
    {
      if(bytes.size() < (56+offset))
        throw std::system_error(EINVAL,std::system_category(),"TOCItem(const std::vector<uint8_t>&): provided vector is too small");
      
      memcpy((void*)(this),bytes.data()+offset,56);
      
      if(bytes.size()< (offset+56+this->path_size))
        throw std::system_error(EINVAL,std::system_category(),"TOCItem(const std::vector<uint8_t>&): provided vector is too small");        
      
      std::string tmp(path_size,'\0');
   
      memcpy((void*)(tmp.data()),bytes.data()+56+offset,path_size);
      relativePath=tmp;
    }
    TOCItem()=delete;
    TOCItem(const TOCItem&)=delete;
    TOCItem(TOCItem&)=delete;
    
    void calcHashSum(const std::vector<uint8_t>& content)
    {
      CryptoPP::SHA256 sha256;
      sha256.CalculateDigest(hashsum,content.data(),content.size());
    }
    void dump(std::vector<uint8_t>& out)
    {
      out.resize(56+relativePath.u8string().size());
      memcpy(out.data(),this,56);
      memcpy(out.data()+56,relativePath.u8string().data(),relativePath.u8string().size());
    }
  };
}


#endif /* __TOC_H__ */

