/*
 * Copyright Pavel Kraynyukhov 2018.
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 *             http://www.boost.org/LICENSE_1_0.txt)
 *
 *  $Id: lar:LAR.h Jun 16, 2018 1:04:08 PM $
 *
 *  EMail: pavel.kraynyukhov@gmail.com
 */


#ifndef __LAR_H__
#define __LAR_H__

#include <fstream>
#include <cassert>
#include <vector>
#include <wolfCryptHaCW.h>
#include <TOC.h>

namespace lar
{
  struct LARHeader
  {
    const char   magic[3];
    const uint8_t version;
    uint32_t TOCSize;
    uint32_t TOCCompressedSize;
    uint32_t TOCItems;
    wolf::SHA256SUM hashsum;
    
    LARHeader() : magic{'L','A','R'},version{1},
    TOCSize{0},TOCCompressedSize{0},TOCItems{0},hashsum{0}{};
    
    const bool valid() const
    {
      if((magic[0] == 'L')&&(magic[1]=='A')&&(magic[2]=='R')&&(version == 1))
        return true;
      return false;
    }
  };
  
  struct LAR
  {
    LARHeader                             header;
    std::vector<std::shared_ptr<TOCItem>> items;
    
    explicit LAR():header(),items(){}
    LAR(const LAR&)=delete;
    LAR(LAR&)=delete;
    
    void add(const std::string& dir_name)
    {
      for(auto dentry: fs::recursive_directory_iterator(dir_name))
      {
        if(dentry.status().type() == fs::file_type::regular)
        {
          items.push_back(std::make_shared<TOCItem>(dentry.path()));
          std::cout << "New archive entry: " << dentry.path() << std::endl;
        }
      }
    }
    
    const bool read_toc(const std::string& archname)
    {
      items.clear();
      std::ifstream archive(archname,std::ios_base::in|std::ios_base::binary);
      if(archive&&archive.is_open())
      {
        archive.read((char*)(&header),sizeof(header));
        assert(archive.gcount() == sizeof(header));
        
        assert(header.valid());
        
        auto compressedTOC=std::make_shared<itc::ByteArray>(header.TOCCompressedSize);
        
        archive.read((char*)(compressedTOC->data()),header.TOCCompressedSize);
        
        auto tocUnCompressed=std::make_shared<itc::ByteArray>();
        
        itc::bz2::decompress(compressedTOC,tocUnCompressed);
        
        size_t offset=0;
        
        do{
          items.push_back(std::make_shared<TOCItem>(*tocUnCompressed,offset));
          offset+=items[items.size()-1]->path_size+56;
        }while(offset < tocUnCompressed->size());
        archive.close();
        return true;
      }
      return false;
    }
    void list()
    {
      for(auto item : items)
      {
        std::cout << item->relativePath << std::endl;
      }
    }
    
    void unpack(const std::string& archname, const fs::path& p=fs::current_path())
    {
      if(read_toc(archname))
      {
        size_t archive_offset=sizeof(header)+header.TOCCompressedSize;
        
        std::ifstream archive(archname,std::ios_base::in|std::ios_base::binary);
        if(archive&&archive.is_open())
        {
          archive.seekg(archive_offset);
          
          for(auto& item : items)
          {
            itc::CompressionBuffer arhived_file=std::make_shared<itc::ByteArray>(item->compressedSize);
            archive.read((char*)(arhived_file->data()),item->compressedSize);
            itc::CompressionBuffer unpacked_file=std::make_shared<itc::ByteArray>(item->fsize);
            itc::bz2::decompress(arhived_file,unpacked_file);

            wolf::SHA256SUM hashsum;
            wolf::sha256sum(*unpacked_file,hashsum);
            if(memcmp(hashsum,item->hashsum,32) == 0)
            {
              fs::path file_path=p / item->relativePath;
              fs::path subdir_path=file_path.parent_path();
              std::error_code ec;
              fs::create_directories(subdir_path,ec);
              
              if(ec.value() != 0)
              {
                throw std::system_error(ec.value(),ec.category(),"Can not create directory: "+subdir_path.u8string()+". Error: "+ec.message());
              }
              
              std::ofstream thefile(file_path,std::ios_base::out|std::ios_base::binary);
              if(thefile&&thefile.is_open())
              {
                thefile.write((const char*)(unpacked_file->data()),unpacked_file->size());
                if(thefile.bad())
                {
                  throw std::system_error(EACCES,std::system_category(),"Can not unpack file: "+file_path.u8string());
                }
                thefile.close();
              }else{
                throw std::system_error(EACCES,std::system_category(),"Can not unpack file: "+file_path.u8string());
              }
            }
            else{
              throw std::system_error(EILSEQ,std::system_category(),"Corrupt archive, hashsum is incorrect");
            }
          }
        }
      }else{
        throw std::system_error(EINVAL,std::system_category(),"Can't read TOC");
      }
    }
    
    void pack(const std::string& archname)
    {
      std::vector<uint8_t>   checksums;
      itc::CompressionBuffer prepin=std::make_shared<itc::ByteArray>();
      header.TOCItems=items.size();
      std::string tempname("TMP_LAR_XXXXXX");
      
      int ffd=mkstemp((char*)(tempname.data()));
      
      
      if(ffd == -1)
        throw std::system_error(errno,std::system_category(),"Can't create temporary file");

      std::string tempname1="TMP_LAR_XXXXXX";
     
      
      int fd=mkstemp((char*)(tempname1.data()));

      if(fd == -1)
        throw std::system_error(errno,std::system_category(),"Can't create temporary file");
      
      for(auto item : items)
      {
        std::ifstream file(item->relativePath,std::ios_base::in|std::ios_base::binary);
        // file.open(item->relativePath,std::ios_base::in|std::ios_base::binary);
        if(file&&file.is_open())
        {
          if(item->fsize > memavail()/3)
          {
            throw std::system_error(
              ENOMEM,
              std::system_category(),
              std::string("File [")+
                item->relativePath.u8string()+
                std::string("] size is too large (larger then the third of available memory)")
            );
          }
          
          itc::CompressionBuffer filebuff=std::make_shared<itc::ByteArray>(item->fsize);
          file.read((char*)filebuff->data(),item->fsize);
          item->calcHashSum(*filebuff);
          checksums.resize(checksums.size()+32);
          memcpy(checksums.data()+(checksums.size()-32),item->hashsum,32);    
          
          itc::CompressionBuffer  compressedContent=std::make_shared<itc::ByteArray>();
          itc::bz2::compress(filebuff,compressedContent);
          item->compressedSize=compressedContent->size();
          auto ret=::write(fd,compressedContent->data(),compressedContent->size());
          if(ret == -1)
          {
            throw std::system_error(
              errno,
              std::system_category(),
              std::string("File [")+
                item->relativePath.u8string()+
                std::string("] error on writing to archive ")+archname
            );
          }
        }
        else{
          throw std::system_error(
            EACCES,
            std::system_category(),
            std::string("Can't open file: ")+item->relativePath.u8string()
          );
        }
        file.close();
      }
      
      wolf::sha256sum(checksums,header.hashsum);
      
      itc::CompressionBuffer TOC=std::make_shared<itc::ByteArray>();
      
      for(auto& item : items)
      {
        std::vector<uint8_t> toc_item;
        item->dump(toc_item);
        TOC->reserve(TOC->size()+toc_item.size());
        TOC->insert(TOC->end(),toc_item.begin(),toc_item.end());
      }
      header.TOCSize=TOC->size();
      itc::CompressionBuffer  compressedTOC=std::make_shared<itc::ByteArray>();
      itc::bz2::compress(TOC,compressedTOC);
      header.TOCCompressedSize=compressedTOC->size();
      
      auto ret=::write(ffd,compressedTOC->data(),compressedTOC->size());
      if(ret == -1)
      {
        throw std::system_error(
          errno,
          std::system_category(),
          std::string("error on writing TOC to archive ")+archname
        );
      }
      
      std::ofstream archive(fs::current_path() / archname, std::ios_base::out|std::ios_base::binary);
      
      if(archive&&archive.is_open())
      {
        archive.write((char*)(&header),sizeof(header));
        // determine TOC size
        lseek(ffd,0,SEEK_SET);
        off_t buff_len=lseek(ffd,0,SEEK_END);

        // read TOC from temp file
        lseek(ffd,0,SEEK_SET);
        char buff[buff_len];
        auto ret=::read(ffd,buff,buff_len);
        if(ret == -1)
        {
          throw std::system_error(
            errno,
            std::system_category(),
            std::string("error on reading from archive ")+archname
          );
        }
        
        // write TOC into archive.
        archive.write(buff,buff_len);
        
        // determine content size:
        lseek(fd,SEEK_SET,0);
        off_t content_buff_len=lseek(fd,0,SEEK_END);
        
        // read content and write it into archive.
        lseek(fd,0,SEEK_SET);
        if(content_buff_len < 4096)
        {
          char nbuff[content_buff_len];
          auto ret=::read(fd,nbuff,content_buff_len);
          if(ret == -1)
          {
            throw std::system_error(
              errno,
              std::system_category(),
              std::string("error on reading from archive ")+archname
            );
          }
          archive.write(nbuff,content_buff_len);
          
        }else{
          char nbuff[4096];
          for(size_t i=0;i<static_cast<size_t>(content_buff_len/4096);++i)
          {
            auto ret=::read(fd,nbuff,4096);
            if(ret == -1)
            {
              throw std::system_error(
                errno,
                std::system_category(),
                std::string("error on reading from archive ")+archname
              );
            }
            archive.write(nbuff,4096);
          }
          size_t last_chunk=(content_buff_len-static_cast<size_t>(content_buff_len/4096)*4096);
          auto ret=::read(fd,nbuff,last_chunk);
          if(ret == -1)
          {
            throw std::system_error(
              errno,
              std::system_category(),
              std::string("error on reading from archive ")+archname
            );
          }
          archive.write(nbuff,last_chunk);
        }
        close(fd);
        close(ffd);
        unlink(tempname.c_str());
        unlink(tempname1.c_str());
        archive.close();
      }
    }
  };
}
#endif /* __LAR_H__ */

