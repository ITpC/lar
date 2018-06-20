/*
 * Copyright Pavel Kraynyukhov 2018.
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 *             http://www.boost.org/LICENSE_1_0.txt)
 *
 *  $Id: lar:main.cpp Jun 16, 2018 1:04:08 PM $
 *
 *  EMail: pavel.kraynyukhov@gmail.com
 */


#include <cstdlib>
#include <iostream>
#include <TOC.h>
#include <LAR.h>

/*
 * 
 */
int main(int argc, char** argv)
{
    static const std::string usage("\nUsage:\n# lar a directory - to add a content of directory into archive recursively\n||\n# lar e archive_name [target subdir] - to extract all files from archive. Where target subdir is a relative path within working directory\n");

    if((argc<3)||(strlen(argv[1]) != 1)||((argv[1][0] != 'e') && (argv[1][0] != 'a')))
    {
        throw std::system_error(EINVAL,std::system_category(),usage);
    }

    switch(argv[1][0])
    {
        case 'a':{
            lar::LAR newarch;
            std::string dirname(argv[2]);
            newarch.add(dirname);
            newarch.pack(dirname+".lar");
        }
        break;
        case 'e':{
            fs::path target_subdir=fs::current_path();
            if(argc==4)
            {
                target_subdir=target_subdir / std::string(argv[3]);
            }
            std::string archname(argv[2]);
            lar::LAR listarch;
            listarch.unpack(archname,target_subdir);
        }
    }

    return 0;
}

