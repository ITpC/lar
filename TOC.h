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
#include <wolfCryptHaCW.h>
#include <bz2Compression.h>
#include <fstream>

#include <utils.h>

namespace fs = std::experimental::filesystem::v1;

namespace lar{
  
  struct TOCItem
  {
    size_t                  fsize;
    size_t                  compressedSize;
    uint32_t                path_size;
    fs::perms               permissions;
    wolf::SHA256SUM         hashsum;
    fs::path                relativePath;
    
    explicit TOCItem(const fs::path& _path)
    : fsize(fs::file_size(_path)),
      compressedSize(0),
      path_size(_path.u8string().length()),
      permissions(fs::status(_path).permissions()),
      hashsum{0,},
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
      wolf::sha256sum(content,hashsum);
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

