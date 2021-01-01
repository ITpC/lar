/**
 *  Copyright 2017-2018, Pavel Kraynyukhov <pavel.kraynyukhov@gmail.com>
 * 
 *  This file is a part of LAppS (Lua Application Server).
 *  
 *  LAppS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  LAppS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with LAppS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  $Id: wolfCryptHaCW.h December 18, 2020 7:48 PM $
 * 
 **/

// wolfCrypt and wolfSSL hash and coding functions

#ifndef __WOLFCRYPTHACW_H__
#  define __WOLFCRYPTHACW_H__

#include <wolfssl/wolfcrypt/sha.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/coding.h>
#include <string_view>
#include <string>
#include <vector>


struct wolf
{
  typedef byte SHA1SUM[SHA_DIGEST_SIZE];
  typedef byte SHA256SUM[SHA256_DIGEST_SIZE];
  
  static auto base64encode(const std::string_view src, std::string& out)
  {
    uint32_t osz=(src.size()*4)/3+10;
    out.resize(osz);
    auto ret=Base64_Encode((const byte*)(src.data()),src.length(),(byte*)(out.data()),&osz);
    out.resize(osz-1);
    return ret;
  }

  
  static auto base64encode(const std::vector<uint8_t>& src, std::vector<uint8_t>& out)
  {
    uint32_t osz=(src.size()*4)/3+10;
    out.resize(osz);
    auto ret=Base64_Encode((const byte*)(src.data()),src.size(),(byte*)(out.data()),&osz);
    out.resize(osz-1);
    return ret;
  }
  
  
  static auto base64encode(const std::vector<uint8_t>& src, std::string& out)
  {
    uint32_t osz=(src.size()*4)/3+10;
    out.resize(osz);
    auto ret=Base64_Encode((const byte*)(src.data()),src.size(),(byte*)(out.data()),&osz);
    out.resize(osz-1);
    return ret;
  }
  

  static void sha1digest(const std::vector<uint8_t>& src, std::vector<uint8_t>& sum)
  {
    SHA1SUM digest;
    sum.resize(SHA_DIGEST_SIZE);
    Sha sha;
    wc_InitSha(&sha);
    wc_ShaUpdate(&sha, (const byte*)(src.data()), src.size());
    wc_ShaFinal(&sha, digest);
    ::memcpy(sum.data(),digest,SHA_DIGEST_SIZE);
  }
  static void sha1digest(const std::string& src, std::vector<uint8_t>& sum)
  {
    SHA1SUM digest;
    sum.resize(SHA_DIGEST_SIZE);
    Sha sha;
    wc_InitSha(&sha);
    wc_ShaUpdate(&sha, (const byte*)(src.data()), src.length());
    wc_ShaFinal(&sha, digest);
    ::memcpy(sum.data(),digest,SHA_DIGEST_SIZE);
  }
  
  static void sha1digest(const std::string& src, SHA1SUM& sum)
  {
    Sha sha;
    wc_InitSha(&sha);
    wc_ShaUpdate(&sha, (const byte*)(src.data()), src.length());
    wc_ShaFinal(&sha, sum);
  }
  
  

  
  static void sha256sum(const std::vector<uint8_t>& src, std::vector<uint8_t>& sum)
  {
    SHA256SUM digest;
    sum.resize(SHA256_DIGEST_SIZE);
    Sha256 sha;
    wc_InitSha256(&sha);
    wc_Sha256Update(&sha, (const byte*)(src.data()), src.size());
    wc_Sha256Final(&sha, digest);
    ::memcpy(sum.data(),digest,SHA256_DIGEST_SIZE);
  }
  static void sha256sum(const std::string& src, std::vector<uint8_t>& sum)
  {
    SHA256SUM digest;
    sum.resize(SHA256_DIGEST_SIZE);
    Sha256 sha;
    wc_InitSha256(&sha);
    wc_Sha256Update(&sha, (const byte*)(src.data()), src.length());
    wc_Sha256Final(&sha, digest);
    ::memcpy(sum.data(),digest,SHA_DIGEST_SIZE);
  }
  
  static void sha256sum(const std::string& src, SHA256SUM& sum)
  {
    Sha256 sha;
    wc_InitSha256(&sha);
    wc_Sha256Update(&sha, (const byte*)(src.data()), src.length());
    wc_Sha256Final(&sha, sum);
  }
  
  static void sha256sum(const std::vector<uint8_t>& src, SHA256SUM& sum)
  {
    Sha256 sha;
    wc_InitSha256(&sha);
    wc_Sha256Update(&sha, (const byte*)(src.data()), src.size());
    wc_Sha256Final(&sha, sum);
  }
  
};

#endif /* __WOLFCRYPTHACW_H__ */

