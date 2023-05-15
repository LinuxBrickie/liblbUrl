#ifndef LIB_LB_URL_MIME_MIMEPART_H
#define LIB_LB_URL_MIME_MIMEPART_H

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

#include <functional>
#include <string>
#include <vector>


namespace lb
{


namespace url
{


namespace mime
{


using Headers = std::vector<std::string>;


struct MimePart
{
  /** \brief Explicitly set the MIME type if the default for the protocel is
             not sufficient.

      From the licurl documentation:

      In the absence of a mime type and if needed by the protocol specifications,
      a default MIME type is determined by the context:
      - If set as a custom header, use this value.
      - application/form-data for an HTTP form post.
      - If a remote file name is set, the MIME type is taken from the file name
        extension, or application/octet-stream by default.
      - For a multipart part, multipart/mixed.
      - text/plain in other cases.
   */
  std::string type;

  /** \brief EXplicitly set the encoding if required.

      Supported values are:
      - "binary"
      - "8bit"
      - "7bit"
      - "base64"
      - "quoted-printable" (not compatible with IMAP)
   */
  std::string encoding;

  /** \brief The name part of the {name, data} pair. */
  std::string name;

  // Use either data OR dataReader

  /** \brief The data to be sent. This will be copied before sending.

      Does not have to be null terminated.

      If this is memory intensive then \sa dataReader.
   */
  std::string data;

  /** \brief A means of specyfying the data to be sent on demand rather than
             up front. This avoids making a copy of the data.

      The full size of the data must be specified in \a totalNumBytes. The
      \a dataReadFn may be invoked more than once to get the total data.

      This is an alternative to \sa data.
   */
  struct DataReader
  {
    /** \brief Data read callback requesting \a buffer to be filled with \a numBytes.
        \return Number of bytes actually written to \a buffer, \a AbortRead or \a PauseRead.

        The callback will be invoked as many times as necessary to get
        \a totalNumBytes. You will never be asked to read the number of bytes
        corresponding to \a AbortRead or \a PauseRead ;)

        You must (obviously) keep track of where you are in the read so you can
        resume in the next call. \sa
     */
    using DataReadFunction = std::function< size_t( char* buffer, size_t numBytes ) >;
    DataReadFunction dataReadFn;

    /** \brief Data seek callback to set where the next read should come from.
        \return Number of bytes actually written to \a buffer, \a AbortRead or \a PauseRead.

        The \a origin will be one of \a seekOriginSet, \a seekOriginCur or
        \a seekOriginEnd which respectively mean start of the file, the current
        position, or end-of-file. In practice libcurl only supports Set.

        You should already be keeping track of where you are in the read for
        the read callback so this will simply prod you to update that location.
     */
    using DataSeekFunction = std::function< int( size_t offset, int origin ) >;
    DataSeekFunction dataSeekFn;

    size_t totalNumBytes{ 0 };

    static const size_t rcReadAbort;
    static const size_t rcReadPause;

    static const int seekOriginSet;
    static const int seekOriginCur;
    static const int seekOriginEnd;

    static const size_t rcSeekOk;
    static const size_t rcSeekFail;
    static const size_t rcSeekCantSeek;
  } dataReader;

  Headers headers;
};


struct Mime
{
  std::vector<MimePart> parts;
};


} // End of namespace mime


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_MIME_MIMEPART_H
