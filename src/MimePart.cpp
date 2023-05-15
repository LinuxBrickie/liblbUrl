/*
    Copyright (C) 2023  Paul Fotheringham (LinuxBrickie)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <lb/url/mime/MimePart.h>

#include "curl/curl.h"


namespace lb
{


namespace url
{


namespace mime
{


const size_t MimePart::DataReader::rcReadAbort{ CURL_READFUNC_ABORT };
const size_t MimePart::DataReader::rcReadPause{ CURL_READFUNC_PAUSE };

const int MimePart::DataReader::seekOriginSet{ SEEK_SET };
const int MimePart::DataReader::seekOriginCur{ SEEK_CUR };
const int MimePart::DataReader::seekOriginEnd{ SEEK_END };

const size_t MimePart::DataReader::rcSeekOk{ CURL_SEEKFUNC_OK };
const size_t MimePart::DataReader::rcSeekFail{ CURL_SEEKFUNC_FAIL };
const size_t MimePart::DataReader::rcSeekCantSeek{ CURL_SEEKFUNC_CANTSEEK };


} // End of namespace mime


} // End of namespace url


} // End of namespace lb
