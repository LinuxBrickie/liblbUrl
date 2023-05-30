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

#include <gtest/gtest.h>

#include <lb/url/http/UrlEncodedValuesCreator.h>


TEST(Http, UrlEncodedValuesCrator)
{
  // Empty field not allowed whether it is encoded or not.
  {
    lb::url::http::UrlEncodedValuesCreator creator;
    EXPECT_FALSE( creator.add( { "" }, { "blah" } ) );
    EXPECT_FALSE( creator.add( { "", false }, { "blah" } ) );
    EXPECT_TRUE ( creator.str().empty() );
  }

  // Test single field with no value.
  {
    lb::url::http::UrlEncodedValuesCreator creator;
    EXPECT_TRUE( creator.add( { "fruit" }, {} ) );
    EXPECT_EQ( creator.str(), std::string{ "fruit=" } );
  }

  // Test single field with unencoded value.
  {
    lb::url::http::UrlEncodedValuesCreator creator;
    EXPECT_TRUE( creator.add( { "fruit" }, { "unencoded", false } ) );
    EXPECT_EQ( creator.str(), std::string{ "fruit=unencoded" } );
  }

  // Test single field with encoded value that needs no encoding.
  {
    lb::url::http::UrlEncodedValuesCreator creator;
    EXPECT_TRUE( creator.add( { "fruit" }, { "encoded" } ) );
    EXPECT_EQ( creator.str(), std::string{ "fruit=encoded" } );
  }

  // Test single field with encoded value that needs encoding.
  {
    lb::url::http::UrlEncodedValuesCreator creator;
    EXPECT_TRUE( creator.add( { "fruit" }, { "encoded!\"£$%^&*(){}[]=+;'#:@,/<>?\\|'" } ) );
    EXPECT_EQ( creator.str(), std::string{ "fruit=encoded%21%22%C2%A3%24%25%5E%26%2A%28%29%7B%7D%5B%5D%3D%2B%3B%27%23%3A%40%2C%2F%3C%3E%3F%5C%7C%27" } );
  }

  // Test multiple fields.
  {
    lb::url::http::UrlEncodedValuesCreator creator;
    EXPECT_TRUE( creator.add( { "fruit" }, { "apple" } ) );
    EXPECT_TRUE( creator.add( { "vegetable" }, { "potato", false } ) );
    EXPECT_TRUE( creator.add( { "empty" }, {} ) );
    EXPECT_TRUE( creator.add( { "encoded-veg" }, { "pot&to" } ) );
    EXPECT_EQ( creator.str(), std::string{ "fruit=apple&vegetable=potato&empty=&encoded-veg=pot%26to" } );
  }

  // Test clear().
  {
    lb::url::http::UrlEncodedValuesCreator creator;
    EXPECT_TRUE( creator.add( { "fruit" }, { "apple" } ) );
    EXPECT_TRUE( creator.add( { "vegetable" }, { "potato", false } ) );
    EXPECT_EQ( creator.str(), std::string{ "fruit=apple&vegetable=potato" } );
    creator.clear();
    EXPECT_TRUE( creator.str().empty() );
  }
}
