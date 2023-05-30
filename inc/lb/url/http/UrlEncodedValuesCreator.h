#ifndef LIB_LB_URL_HTTP_URLENCODEDVALUESCREATOR_H
#define LIB_LB_URL_HTTP_URLENCODEDVALUESCREATOR_H

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

#include <sstream>


namespace lb
{


namespace url
{


namespace http
{


class UrlEncodedValuesCreator
{
public:
  /** \brief A string which can be marked as already encoded or not.

      This is used for both field names and values allowing you to specify
      whether you have already encoded the string (or it doesn't need encoding)
      or whether you wish it to be encoded for you.
   */
  struct Encodable
  {
    Encodable() = default;

    std::string s;
    bool needsEncoded{ true };
  };

  /** \brief Add a {field, value} pair, optionally encoding them.
             Value may be empty but field must not.
      \return False if the field is empty or the encoding failed.
  */
  bool add( const Encodable& field, const Encodable& value );

  /** \brief Return the full string as it currently stands. */
  std::string str() const;

  /** \brief Reset the string back to the empty string. */
  void clear();

private:
  std::stringstream ss;
  bool empty{ true };

  bool addEncodable( const Encodable& );
};


} // End of namespace http


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_HTTP_URLENCODEDVALUESCREATOR_H
