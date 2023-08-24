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

#include "TestWsRequester.h"

#include <gtest/gtest.h>

#include <future>
#include <thread>

#include <lb/url/Requester.h>

#include "ServerList.h"

const std::string clientCloseReason{ "Client initiating close" };
const std::string serverCloseReason{ "Server initiating close" };


const std::unordered_map<std::string, ExpectedResponse> WSExpectedMockResponses
{
  {
    "/test/url/ws/invalid",
    {
      lb::url::ResponseCode::eFailure,
    }
  },
  {
    "/test/url/ws/hello",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello1",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello2",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello3",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello4",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello/name",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } }
      , { { "What's your name?" }, { "Paul" } } }
    }
  },
  {
    "/test/url/ws/goodbye",
    {
      lb::url::ResponseCode::eSuccess,
      { { sendControlCloseMessage, serverCloseReason } }
    }
  }
};

struct ChallengeConnection
{
  ChallengeConnection( lb::url::Requester& requester
                     , const std::string& url
                     , const ExpectedResponse& expectedResponse )
    : requester{ requester }
    , url{ url }
    , expectedResponse{ expectedResponse }
  {
  }

  void operator()( int port )
  {
    const size_t numExpectedResponses{ expectedResponse.responseMessages.size() };

    using EstablishedPromise = std::promise< std::pair<lb::url::ResponseCode, lb::url::ws::Response> >;
    using DataRcvPromise     = std::promise<std::string>;
    using ControlRcvPromise  = std::promise< std::pair<lb::url::ws::ControlOpCode, std::string> >;

    EstablishedPromise ep;
    std::vector<DataRcvPromise> drps;
    drps.resize( numExpectedResponses );
    ControlRcvPromise crp;
    size_t responseIndex{ 0 };

    requester.makeRequest( {
                             "ws://" + hostColonPort( port ) + url
                           , [&drps, responseIndex, numExpectedResponses]( lb::url::ws::ConnectionID
                                                                         , lb::url::ws::DataOpCode
                                                                         , std::string r ) mutable // data receiver
                             {
                               ASSERT_LT( responseIndex, numExpectedResponses );
                               drps[responseIndex].set_value( std::move( r ) );
                               ++responseIndex;
                             }
                           , [&crp]( lb::url::ws::ConnectionID
                                   , lb::url::ws::ControlOpCode c
                                   , std::string r )
                             {
                               crp.set_value( { c, std::move( r ) } );
                             }
                           }
                         , [&ep]( lb::url::ResponseCode rc, lb::url::ws::Response r )
                           {
                             ep.set_value( { rc, std::move( r ) } );
                           } );

    const auto establishedPair{ ep.get_future().get() };
    const lb::url::ResponseCode actualResponseCode{ establishedPair.first };
    const lb::url::ws::Response& actualResponse{ establishedPair.second };
    EXPECT_EQ( actualResponseCode, expectedResponse.responseCode );

    // Connection established
    bool serverAskedToClose{ false };

    if ( actualResponseCode == lb::url::ResponseCode::eSuccess )
    {
      size_t i = 0;
      for ( const auto&[challenge, expectedMessage] : expectedResponse.responseMessages )
      {
        const auto sendResult{ actualResponse.senders.sendData( lb::url::ws::DataOpCode::eText, challenge ) };
        EXPECT_EQ( sendResult, lb::url::ws::SendResult::eSuccess );

        std::string actualMessage;
        if ( challenge == sendControlCloseMessage )
        {
          // A data request that asks the server to close the connection. This
          // should always be the final challenge.

          // Following two are asserts because if they fail they indicate that
          // the test challenges have been set up incorrectly.
          ASSERT_EQ( i, expectedResponse.responseMessages.size() - 1 );
          ASSERT_EQ( serverCloseReason, expectedResponse.responseMessages.rbegin()->second );
          serverAskedToClose = true;
        }
        else
        {
          actualMessage = drps[i].get_future().get();
          EXPECT_EQ( actualMessage, expectedMessage );
          ++i;
        }
      }

      if ( !serverAskedToClose )
      {
        auto sendResult
        {
          actualResponse.senders.sendClose( lb::encoding::websocket::closestatus::toPayload(
                                              lb::encoding::websocket::closestatus::ProtocolCode::eNormal )
                                          , clientCloseReason )
        };
        EXPECT_EQ( sendResult, lb::url::ws::SendResult::eSuccess );
      }

      const auto actualControlPair{ crp.get_future().get() };
      EXPECT_EQ( actualControlPair.first, lb::url::ws::ControlOpCode::eClose );
      ASSERT_TRUE( actualControlPair.second.size() >= 2 );
      const auto actualStatusCode
      {
        lb::encoding::websocket::closestatus::decodePayloadCode( actualControlPair.second )
      };
      EXPECT_EQ( actualStatusCode
               , lb::encoding::websocket::closestatus::toPayload(
                   lb::encoding::websocket::closestatus::ProtocolCode::eNormal ) );
      EXPECT_EQ( actualControlPair.second.substr( 2 ), serverAskedToClose ? serverCloseReason : clientCloseReason );

      // Should not be able to send anything now.
      auto sendResult = actualResponse.senders.sendData( lb::url::ws::DataOpCode::eText, "blah" );
      EXPECT_EQ( sendResult, lb::url::ws::SendResult::eClosed );
      sendResult = actualResponse.senders.sendClose( lb::encoding::websocket::closestatus::toPayload(
                                                       lb::encoding::websocket::closestatus::ProtocolCode::eNormal ) );
      EXPECT_EQ( sendResult, lb::url::ws::SendResult::eClosed );
    }
    else if ( actualResponseCode == lb::url::ResponseCode::eFailure )
    {
      // Should not be able to send anything.
      auto sendResult = actualResponse.senders.sendData( lb::url::ws::DataOpCode::eText, "blah" );
      EXPECT_EQ( sendResult, lb::url::ws::SendResult::eNoImplementation );
      sendResult =
          actualResponse.senders.sendClose( lb::encoding::websocket::closestatus::toPayload(
                                              lb::encoding::websocket::closestatus::ProtocolCode::eNormal )
                                          , "Test finished" );
      EXPECT_EQ( sendResult, lb::url::ws::SendResult::eNoImplementation );
    }
  }

  lb::url::Requester& requester;
  const std::string url;
  const ExpectedResponse expectedResponse;
};

TEST(Ws, Requester_Challenges_Serial)
{
  lb::url::Requester requester{ { 1000 } };

  const auto& serverConfigs = serverList.at( httpd::ServerType::eWebSocket );
  for ( const auto serverConfig : serverConfigs )
  {
    for ( const auto&[ url, challengeExpectedResponses ] : WSExpectedMockResponses )
    {
      ChallengeConnection{ requester, url, challengeExpectedResponses }( serverConfig.port );
    }
  }
}

TEST(Ws, Requester_Challenges_Parallel)
{
  lb::url::Requester requester;

  const auto& serverConfigs = serverList.at( httpd::ServerType::eWebSocket );
  for ( const auto& serverConfig : serverConfigs )
  {
    using Threads = std::vector<std::thread>;
    Threads threads{ WSExpectedMockResponses.size() };

    size_t c = 0;
    for ( const auto&[ url, challengeExpectedResponses ] : WSExpectedMockResponses )
    {
      threads[c] = std::move( std::thread{ ChallengeConnection{ requester
                                                              , url
                                                              , challengeExpectedResponses }
                                         , serverConfig.port } );
      ++c;
    }

    for ( auto& thread : threads )
    {
      thread.join();
    }
  }
}

void testRequesterDestruction( int port
                             , size_t pollTimeoutMilliseconds
                             , size_t closeTimeoutInMilliSeconds )
{
  lb::url::Requester requester{ { pollTimeoutMilliseconds } };
  std::promise<lb::url::ws::Response> connectionEstablishedPromise;
  requester.makeRequest( {
                           "ws://" + hostColonPort( port ) + "/test/url/ws/destruction"
                         , []( lb::url::ws::ConnectionID, lb::url::ws::DataOpCode, std::string ) // no-op data receiver
                           {
                           }
                         , []( lb::url::ws::ConnectionID, lb::url::ws::ControlOpCode, std::string ) // no-op control receiver
                           {
                           }
                         , closeTimeoutInMilliSeconds
                         }
                       , [ &connectionEstablishedPromise ]( lb::url::ResponseCode rc, lb::url::ws::Response r )
                         {
                           connectionEstablishedPromise.set_value( std::move( r ) );
                         } );

  lb::url::ws::Response actualResponse{ connectionEstablishedPromise.get_future().get() };
}

TEST(Ws, Requester_Destruction)
{  
  const auto& serverConfigs = serverList.at( httpd::ServerType::eWebSocket );
  for ( const auto& serverConfig : serverConfigs )
  {
    // Start with a quick poll timeout and get successively longer until we reach
    // the default value. The first iteration will not have time to receive the
    // close confirmation from the server whereas the last should.

    // A close timeout shorter than the poll timeout should guarantee that the
    // close confirmation from the server will not be received, thereby testing
    // that the connection is forvibly shutdown correctly.
    testRequesterDestruction( serverConfig.port
                            , 200  // poll timeout
                            , 1 ); // close timeoue

    // A close timeout longer than the poll timeout and of a reasonable size
    // should guarantee that the close confirmation from the server will be
    // received.
    testRequesterDestruction( serverConfig.port
                            , 300     // poll timeout
                            , 1000 ); // close timeout

    // Test closing off the Receivers knowing that data will be sent back
    lb::url::ws::Request request
    {
      "ws://" + hostColonPort( serverConfig.port ) + "/test/url/ws/destruction/with-data"
    , []( lb::url::ws::ConnectionID, lb::url::ws::DataOpCode, std::string r ) // data receiver
      {
      }
    , []( lb::url::ws::ConnectionID, lb::url::ws::ControlOpCode, std::string r )
      {
      }
    };
    auto receivers{ request.receivers };
    // Can do it now before we even make the request. This guarantees they were
    // closed off before any data arrives!
    receivers.stopReceiving();

    lb::url::Requester requester;
    std::promise<lb::url::ws::Response> connectionEstablishedPromise;
    requester.makeRequest( request
                         , [ &connectionEstablishedPromise ]( lb::url::ResponseCode rc, lb::url::ws::Response r )
                           {
                             connectionEstablishedPromise.set_value( std::move( r ) );
                           } );

    lb::url::ws::Response actualResponse{ connectionEstablishedPromise.get_future().get() };

    size_t i = 0;
    const auto sendResult{ actualResponse.senders.sendData( lb::url::ws::DataOpCode::eText, "any old rubbish" ) };
    EXPECT_EQ( sendResult, lb::url::ws::SendResult::eSuccess );

    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
  }
}
